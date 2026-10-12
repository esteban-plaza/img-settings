#include "watermark.h"

#ifdef HAVE_JXL
#include "jxl.h"
#endif

#include <QBuffer>
#include <QCollator>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetricsF>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QPainterPath>
#include <QSet>
#include <QProcess>
#include <QStandardPaths>
#include <QtMath>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace Watermark {

namespace {

QString g_fontFamily;

struct Item {
    QString icon;
    QString text;
    qreal textWidth = 0;
};

QList<Item> buildItems(const PhotoSettings &s)
{
    QList<Item> items;
    if (!s.cameraModel.isEmpty())
        items.append({ u"camera"_s, s.cameraModel });
    if (!s.aperture.isEmpty())
        items.append({ u"aperture"_s, u"f/"_s + s.aperture });
    if (!s.shutterSpeed.isEmpty())
        items.append({ u"shutter"_s, s.shutterSpeed });
    if (!s.iso.isEmpty())
        items.append({ u"iso"_s, u"ISO "_s + s.iso });
    if (!s.focalLength.isEmpty())
        items.append({ u"focal"_s, s.focalLength });
    return items;
}

QColor white(qreal alpha) { return QColor::fromRgbF(1, 1, 1, alpha); }

void strokeWith(QPainter &p, const QColor &color, qreal width)
{
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
}

void fillWith(QPainter &p, const QColor &color)
{
    p.setPen(Qt::NoPen);
    p.setBrush(color);
}

void drawIcon(QPainter &p, const QString &kind, qreal cx, qreal cy, qreal size)
{
    const qreal r = size * 0.42;
    const qreal lw = size * 0.085;
    const QColor fg = white(0.92);

    if (kind == u"aperture") {
        strokeWith(p, fg, lw);
        p.drawEllipse(QPointF(cx, cy), r, r);
        strokeWith(p, fg, lw * 0.85);
        for (int i = 0; i < 6; ++i) {
            const qreal a1 = i * M_PI / 3.0 - M_PI / 6;
            const qreal a2 = a1 + M_PI / 3.0;
            p.drawLine(QPointF(cx + r * 0.30 * qCos(a1), cy + r * 0.30 * qSin(a1)),
                       QPointF(cx + r * 0.88 * qCos(a2), cy + r * 0.88 * qSin(a2)));
        }
        fillWith(p, fg);
        p.drawEllipse(QPointF(cx, cy), r * 0.18, r * 0.18);
    } else if (kind == u"shutter") {
        strokeWith(p, fg, lw);
        p.drawEllipse(QPointF(cx, cy), r, r);
        strokeWith(p, fg, lw * 0.75);
        for (int i = 0; i < 4; ++i) {
            const qreal a = i * M_PI / 2.0 - M_PI / 2;
            p.drawLine(QPointF(cx + r * 0.72 * qCos(a), cy + r * 0.72 * qSin(a)),
                       QPointF(cx + r * 0.95 * qCos(a), cy + r * 0.95 * qSin(a)));
        }
        strokeWith(p, fg, lw);
        p.drawLine(QPointF(cx, cy), QPointF(cx + r * 0.42 * qCos(-2.25), cy + r * 0.42 * qSin(-2.25)));
        p.drawLine(QPointF(cx, cy), QPointF(cx, cy - r * 0.72));
        fillWith(p, fg);
        p.drawEllipse(QPointF(cx, cy), lw * 0.7, lw * 0.7);
    } else if (kind == u"iso") {
        const qreal rw = r * 1.45;
        const qreal rh = r * 1.20;
        strokeWith(p, fg, lw);
        p.drawRoundedRect(QRectF(cx - rw / 2, cy - rh / 2, rw, rh), lw * 1.2, lw * 1.2);
        strokeWith(p, fg, lw * 0.65);
        p.drawLine(QPointF(cx, cy - rh / 2 + lw * 1.8), QPointF(cx, cy + rh / 2 - lw * 1.8));
        p.drawLine(QPointF(cx - rw / 2 + lw * 1.8, cy), QPointF(cx + rw / 2 - lw * 1.8, cy));
        const qreal pinLen = rw * 0.18;
        for (qreal py : { cy - rh * 0.22, cy + rh * 0.22 }) {
            p.drawLine(QPointF(cx - rw / 2 - pinLen, py), QPointF(cx - rw / 2, py));
            p.drawLine(QPointF(cx + rw / 2, py), QPointF(cx + rw / 2 + pinLen, py));
        }
    } else if (kind == u"focal") {
        strokeWith(p, fg, lw);
        p.drawEllipse(QPointF(cx, cy), r * 0.95, r);
        p.drawEllipse(QPointF(cx, cy), r * 0.52, r * 0.52);
        p.drawEllipse(QPointF(cx, cy), r * 0.22, r * 0.22);
        fillWith(p, white(0.55));
        p.drawEllipse(QPointF(cx - r * 0.22, cy - r * 0.30), r * 0.13, r * 0.07);
    } else if (kind == u"camera") {
        strokeWith(p, fg, lw);
        const qreal bw = r * 2.0;
        const qreal bh = r * 1.30;
        const qreal bx = cx - bw / 2;
        const qreal by = cy - bh / 2 + r * 0.10;
        p.drawRoundedRect(QRectF(bx, by, bw, bh), lw * 1.4, lw * 1.4);
        const qreal vw = bw * 0.38;
        const qreal vh = bh * 0.30;
        p.drawRoundedRect(QRectF(bx + bw * 0.12, by - vh + lw * 0.5, vw, vh), lw * 0.8, lw * 0.8);
        p.drawEllipse(QPointF(cx + bw * 0.05, cy + r * 0.12), r * 0.50, r * 0.50);
        p.drawEllipse(QPointF(cx + bw * 0.05, cy + r * 0.12), r * 0.28, r * 0.28);
        fillWith(p, white(0.70));
        p.drawEllipse(QPointF(bx + bw * 0.78, by - vh * 0.10), lw * 1.1, lw * 1.1);
    }
}

// Sony ARW files carry a full-size JPEG preview; use the largest embedded JPEG.
QImage extractLargestJpeg(const QByteArray &data)
{
    static const QByteArray soi("\xFF\xD8\xFF", 3);
    static const QByteArray eoi("\xFF\xD9", 2);
    qsizetype bestStart = -1, bestLen = 0;
    for (qsizetype i = data.indexOf(soi); i >= 0; i = data.indexOf(soi, i + 1)) {
        const qsizetype end = data.indexOf(eoi, i + 4);
        if (end < 0)
            break;
        if (end + 2 - i > bestLen) {
            bestStart = i;
            bestLen = end + 2 - i;
        }
    }
    if (bestStart < 0)
        return {};
    return QImage::fromData(QByteArrayView(data).sliced(bestStart, bestLen), "JPEG");
}

// GUI apps launched from Finder don't inherit the shell PATH, so also look in Homebrew's prefixes.
QString findTool(const QString &name)
{
    QString path = QStandardPaths::findExecutable(name);
    if (path.isEmpty())
        path = QStandardPaths::findExecutable(name, { u"/opt/homebrew/bin"_s, u"/usr/local/bin"_s });
    return path;
}

QImage decodeWithTool(const QString &tool, const QStringList &args)
{
    const QString exe = findTool(tool);
    if (exe.isEmpty())
        return {};
    QProcess proc;
    proc.start(exe, args);
    if (!proc.waitForFinished(120000) || proc.exitCode() != 0)
        return {};
    return QImage::fromData(proc.readAllStandardOutput(), "PPM");
}

QImage decodeArw(const QString &path, const QByteArray &data, QString *error)
{
    QImage img = extractLargestJpeg(data);
    if (img.isNull())
        img = decodeWithTool(u"dcraw"_s, { u"-c"_s, u"-w"_s, u"-q"_s, u"3"_s, path });
    for (const QString tool : { u"convert"_s, u"magick"_s }) {
        if (!img.isNull())
            break;
        img = decodeWithTool(tool, { path, u"ppm:-"_s });
    }
    if (img.isNull() && error)
        *error = u"cannot decode ARW file \"%1\" — install dcraw or ImageMagick"_s.arg(QFileInfo(path).fileName());
    return img;
}

} // namespace

void setFontFamily(const QString &family) { g_fontFamily = family; }

QStringList supportedFormats()
{
    QStringList formats { u"ARW"_s, u"JPG"_s, u"PNG"_s };
#ifdef HAVE_JXL
    formats.append(u"JXL"_s);
#endif
    return formats;
}

bool isSupported(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    return ext == u"arw" || ext == u"jpg" || ext == u"jpeg" || ext == u"png"
#ifdef HAVE_JXL
        || ext == u"jxl"
#endif
        ;
}

QString outputPath(const QString &inputPath, const QString &outDir)
{
    return QDir(outDir).filePath(QFileInfo(inputPath).completeBaseName() + u".jpg");
}

// Dropped folders are scanned for supported photos; subfolders only when recurse is on.
// Files already inside the output folder are skipped so re-runs don't watermark twice.
QStringList collectFiles(const QStringList &paths, bool recurse, const QString &outDir)
{
    const QString outPrefix = QDir::cleanPath(outDir) + u'/';
    QStringList files;
    QSet<QString> seen;
    auto add = [&](const QString &file) {
        const QString clean = QDir::cleanPath(file);
        if (isSupported(clean) && !clean.startsWith(outPrefix) && !seen.contains(clean)) {
            seen.insert(clean);
            files.append(clean);
        }
    };

    for (const QString &path : paths) {
        const QFileInfo info(path);
        if (info.isDir()) {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot,
                            recurse ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
            while (it.hasNext())
                add(it.next());
        } else if (info.isFile()) {
            add(path);
        }
    }

    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(files.begin(), files.end(), collator);
    return files;
}

QImage decodeImage(const QString &path, const QByteArray &data, QString *error)
{
    if (QFileInfo(path).suffix().toLower() == u"arw")
        return decodeArw(path, data, error);

    QBuffer buffer;
    buffer.setData(data);
    QImageReader reader(&buffer);
    reader.setAutoTransform(false); // orientation is applied explicitly from EXIF
    QImage img = reader.read();
    if (img.isNull() && error)
        *error = u"decode: "_s + reader.errorString();
    return img;
}

QImage applyOrientation(const QImage &image, int orientation)
{
    QTransform rotate90;
    rotate90.rotate(90);
    switch (orientation) {
    case 2: return image.flipped(Qt::Horizontal);
    case 3: return image.flipped(Qt::Horizontal | Qt::Vertical);
    case 4: return image.flipped(Qt::Vertical);
    case 5: return image.transformed(rotate90).flipped(Qt::Horizontal);
    case 6: return image.transformed(rotate90);
    case 7: return image.transformed(rotate90).flipped(Qt::Vertical);
    case 8: return image.transformed(QTransform().rotate(270));
    default: return image;
    }
}

QImage resizeForWhatsApp(const QImage &image)
{
    const int w = image.width();
    const int h = image.height();
    if (w <= MaxWhatsAppDim && h <= MaxWhatsAppDim)
        return image;
    const QSize target = w >= h ? QSize(MaxWhatsAppDim, qRound(qreal(h) * MaxWhatsAppDim / w))
                                : QSize(qRound(qreal(w) * MaxWhatsAppDim / h), MaxWhatsAppDim);
    return image.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QImage addWatermark(const QImage &image, const PhotoSettings &settings, double opacity)
{
    QList<Item> items = buildItems(settings);
    if (items.isEmpty())
        return image; // no EXIF data — leave the photo untouched

    const qreal imgW = image.width();
    const qreal imgH = image.height();

    // Font size proportional to image height, clamped.
    int fontSize = qBound(16, int(qRound(imgH / 52.0)), 56);
    const qreal margin = qMax(imgH / 55.0, 14.0);
    const qreal maxBoxW = imgW - 2 * margin;
    constexpr qreal sepW = 1.0;

    QFont font(g_fontFamily);
    font.setHintingPreference(QFont::PreferFullHinting);
    qreal iconSize, padV, padH, textIconGap, itemGap, boxW, boxH, lineHeight;

    // Shrink the font until all items fit within the image width.
    for (;;) {
        font.setPixelSize(fontSize);
        const QFontMetricsF fm(font);
        lineHeight = fm.lineSpacing();
        iconSize = fontSize * 1.25;
        padV = fontSize * 0.65;
        padH = fontSize * 0.90;
        textIconGap = fontSize * 0.32;
        itemGap = fontSize * 0.85;

        boxW = padH * 2;
        for (qsizetype i = 0; i < items.size(); ++i) {
            items[i].textWidth = fm.horizontalAdvance(items[i].text);
            boxW += iconSize + textIconGap + items[i].textWidth;
            if (i < items.size() - 1)
                boxW += itemGap + sepW;
        }
        boxH = iconSize + padV * 2;

        if (boxW <= maxBoxW || fontSize <= 10)
            break;
        --fontSize;
    }

    const qreal ox = (imgW - boxW) / 2;
    const qreal oy = imgH - boxH - margin;

    QImage out = image.convertToFormat(QImage::Format_RGB32);
    QPainter p(&out);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    p.setFont(font);

    // Background pill
    fillWith(p, QColor::fromRgbF(0.06f, 0.06f, 0.08f, float(opacity)));
    p.drawRoundedRect(QRectF(ox, oy, boxW, boxH), boxH / 2, boxH / 2);

    qreal cx = ox + padH;
    const qreal cy = oy + boxH / 2.0;
    for (qsizetype i = 0; i < items.size(); ++i) {
        const Item &it = items[i];
        drawIcon(p, it.icon, cx + iconSize / 2, cy, iconSize);
        cx += iconSize + textIconGap;

        p.setPen(white(0.95));
        p.drawText(QPointF(cx, cy + fontSize * 0.04 + 0.38 * lineHeight), it.text);
        cx += it.textWidth;

        if (i < items.size() - 1) {
            cx += itemGap * 0.45;
            strokeWith(p, white(0.22), sepW);
            p.drawLine(QPointF(cx, oy + boxH * 0.2), QPointF(cx, oy + boxH * 0.8));
            cx += sepW + itemGap * 0.55;
        }
    }
    p.end();
    return out;
}

QString processFile(const QString &inputPath, const QString &outDir, double opacity)
{
    QFile file(inputPath);
    if (!file.open(QIODevice::ReadOnly))
        return u"open: "_s + file.errorString();
    const QByteArray data = file.readAll();
    file.close();

    PhotoSettings settings;
    QString error;
    QImage img;

#ifdef HAVE_JXL
    if (QFileInfo(inputPath).suffix().toLower() == u"jxl") {
        QByteArray exif;
        img = decodeJxl(data, &exif, &error);
        settings = readExif(exif);
        settings.orientation = 1; // libjxl has already applied it
    } else
#endif
    {
        settings = readExif(data);
        img = decodeImage(inputPath, data, &error);
    }
    if (img.isNull())
        return error.isEmpty() ? u"decode failed"_s : error;

    img = applyOrientation(img, settings.orientation);
    img = resizeForWhatsApp(img);
    img = addWatermark(img, settings, opacity);

    QImageWriter writer(outputPath(inputPath, outDir), "jpeg");
    writer.setQuality(JpegQuality);
    if (!writer.write(img))
        return u"write: "_s + writer.errorString();
    return {};
}

} // namespace Watermark
