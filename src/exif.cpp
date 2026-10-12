#include "exif.h"

#include <cmath>
#include <optional>

using namespace Qt::StringLiterals;

namespace {

// Minimal TIFF/EXIF reader: just the handful of tags img-settings needs.
class TiffReader
{
public:
    explicit TiffReader(QByteArrayView tiff) : m_data(tiff)
    {
        if (m_data.size() < 8)
            return;
        if (m_data.startsWith("II"))
            m_littleEndian = true;
        else if (m_data.startsWith("MM"))
            m_littleEndian = false;
        else
            return;
        m_valid = u16(2) == 42;
    }

    bool isValid() const { return m_valid; }
    quint32 firstIfd() const { return u32(4); }

    struct Entry {
        quint16 tag = 0;
        quint16 type = 0;
        quint32 count = 0;
        quint32 valueOffset = 0; // offset of the value bytes inside the TIFF
    };

    template <typename F>
    void forEachEntry(quint32 ifd, F &&fn) const
    {
        if (!inRange(ifd, 2))
            return;
        const quint16 n = u16(ifd);
        for (quint16 i = 0; i < n; ++i) {
            const quint32 at = ifd + 2 + i * 12;
            if (!inRange(at, 12))
                return;
            Entry e { u16(at), u16(at + 2), u32(at + 4), 0 };
            const quint64 size = quint64(typeSize(e.type)) * e.count;
            e.valueOffset = size <= 4 ? at + 8 : u32(at + 8);
            fn(e);
        }
    }

    std::optional<double> rational(const Entry &e) const
    {
        if ((e.type != 5 && e.type != 10) || !inRange(e.valueOffset, 8))
            return std::nullopt;
        const double num = e.type == 5 ? double(u32(e.valueOffset)) : double(qint32(u32(e.valueOffset)));
        const double den = e.type == 5 ? double(u32(e.valueOffset + 4)) : double(qint32(u32(e.valueOffset + 4)));
        if (den == 0)
            return std::nullopt;
        return num / den;
    }

    std::optional<qint64> integer(const Entry &e) const
    {
        if (!inRange(e.valueOffset, typeSize(e.type)))
            return std::nullopt;
        switch (e.type) {
        case 1: case 7: return quint8(m_data[e.valueOffset]);
        case 3: return u16(e.valueOffset);
        case 4: return u32(e.valueOffset);
        case 9: return qint32(u32(e.valueOffset));
        default: return std::nullopt;
        }
    }

    QString ascii(const Entry &e) const
    {
        if (e.type != 2 || !inRange(e.valueOffset, e.count))
            return {};
        QByteArray bytes = m_data.sliced(e.valueOffset, e.count).toByteArray();
        const qsizetype nul = bytes.indexOf('\0');
        if (nul >= 0)
            bytes.truncate(nul);
        return QString::fromLatin1(bytes).trimmed();
    }

private:
    static int typeSize(quint16 type)
    {
        switch (type) {
        case 1: case 2: case 6: case 7: return 1;
        case 3: case 8: return 2;
        case 4: case 9: case 11: return 4;
        case 5: case 10: case 12: return 8;
        default: return 1;
        }
    }

    bool inRange(quint64 offset, quint64 len) const { return offset + len <= quint64(m_data.size()); }

    quint16 u16(quint32 at) const
    {
        if (!inRange(at, 2))
            return 0;
        const auto b0 = quint8(m_data[at]), b1 = quint8(m_data[at + 1]);
        return m_littleEndian ? quint16(b0 | b1 << 8) : quint16(b0 << 8 | b1);
    }

    quint32 u32(quint32 at) const
    {
        if (!inRange(at, 4))
            return 0;
        const quint32 a = u16(at), b = u16(at + 2);
        return m_littleEndian ? (a | b << 16) : (a << 16 | b);
    }

    QByteArrayView m_data;
    bool m_littleEndian = true;
    bool m_valid = false;
};

QByteArrayView findTiff(const QByteArray &data)
{
    // Raw TIFF container (Sony ARW and friends).
    if (data.startsWith("II*\0") || data.startsWith(QByteArrayView("MM\0*", 4)))
        return data;

    // JPEG: look for the APP1 "Exif\0\0" segment before the image data.
    if (data.size() > 4 && quint8(data[0]) == 0xFF && quint8(data[1]) == 0xD8) {
        qsizetype pos = 2;
        while (pos + 4 <= data.size() && quint8(data[pos]) == 0xFF) {
            const quint8 marker = quint8(data[pos + 1]);
            if (marker == 0xDA || marker == 0xD9) // start of scan / end of image
                break;
            const int len = quint8(data[pos + 2]) << 8 | quint8(data[pos + 3]);
            if (len < 2 || pos + 2 + len > data.size())
                break;
            const QByteArrayView payload(data.constData() + pos + 4, len - 2);
            if (marker == 0xE1 && payload.startsWith(QByteArrayView("Exif\0\0", 6)))
                return payload.sliced(6);
            pos += 2 + len;
        }
        return {};
    }

    // PNG: optional eXIf chunk.
    if (data.startsWith("\x89PNG\r\n\x1a\n")) {
        qsizetype pos = 8;
        while (pos + 12 <= data.size()) {
            const quint32 len = quint32(quint8(data[pos])) << 24 | quint8(data[pos + 1]) << 16
                              | quint8(data[pos + 2]) << 8 | quint8(data[pos + 3]);
            const QByteArrayView type(data.constData() + pos + 4, 4);
            if (pos + 12 + qsizetype(len) > data.size())
                break;
            if (type == "eXIf")
                return QByteArrayView(data.constData() + pos + 8, len);
            if (type == "IDAT" || type == "IEND")
                break;
            pos += 12 + len;
        }
    }
    return {};
}

QString trimZero(QString s)
{
    if (s.endsWith(QLatin1String(".0")))
        s.chop(2);
    return s;
}

} // namespace

PhotoSettings readExif(const QByteArray &data)
{
    PhotoSettings s;
    const TiffReader tiff(findTiff(data));
    if (!tiff.isValid())
        return s;

    enum : quint16 {
        Model = 0x0110, Orientation = 0x0112, ExifIfd = 0x8769,
        ExposureTime = 0x829A, FNumber = 0x829D, Iso = 0x8827, FocalLength = 0x920A,
    };

    quint32 exifIfd = 0;
    tiff.forEachEntry(tiff.firstIfd(), [&](const TiffReader::Entry &e) {
        switch (e.tag) {
        case Model: s.cameraModel = tiff.ascii(e); break;
        case Orientation: s.orientation = int(tiff.integer(e).value_or(1)); break;
        case ExifIfd: exifIfd = quint32(tiff.integer(e).value_or(0)); break;
        }
    });

    if (exifIfd == 0)
        return s;

    tiff.forEachEntry(exifIfd, [&](const TiffReader::Entry &e) {
        switch (e.tag) {
        case FNumber:
            if (auto v = tiff.rational(e))
                s.aperture = trimZero(QString::number(*v, 'f', 1));
            break;
        case ExposureTime:
            if (auto v = tiff.rational(e); v && *v > 0) {
                if (*v >= 1)
                    s.shutterSpeed = QString::number(*v, 'f', *v == std::floor(*v) ? 0 : 1) + u"s"_s;
                else
                    s.shutterSpeed = QStringLiteral("1/%1").arg(qRound(1.0 / *v));
            }
            break;
        case Iso:
            if (auto v = tiff.integer(e))
                s.iso = QString::number(*v);
            break;
        case FocalLength:
            if (auto v = tiff.rational(e))
                s.focalLength = QString::number(*v, 'f', *v == std::floor(*v) ? 0 : 1) + u"mm"_s;
            break;
        }
    });

    return s;
}
