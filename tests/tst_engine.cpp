#include "exif.h"
#include "watermark.h"

#ifdef HAVE_JXL
#include "jxl.h"
#endif

#include <QBuffer>
#include <QFile>
#include <QFontDatabase>
#include <QImageReader>
#include <QImageWriter>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

using namespace Qt::StringLiterals;

namespace {

QByteArray encodeJpeg(const QImage &image)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    QImageWriter writer(&buffer, "jpeg");
    writer.write(image);
    return bytes;
}

QImage solidImage(int w, int h, QColor color = Qt::darkCyan)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(color);
    return img;
}

// Little-endian TIFF with IFD0 {Model, Orientation, ExifIFD} and an Exif IFD with
// ExposureTime 1/250, FNumber 2.8, ISO 400 and FocalLength 35.
QByteArray exifTiff(quint16 orientation)
{
    QByteArray tiff;
    auto u16 = [&](quint16 v) { tiff.append(char(v & 0xFF)).append(char(v >> 8)); };
    auto u32 = [&](quint32 v) { u16(quint16(v & 0xFFFF)); u16(quint16(v >> 16)); };
    auto entry = [&](quint16 tag, quint16 type, quint32 count, quint32 value) {
        u16(tag); u16(type); u32(count); u32(value);
    };

    const QByteArray model("ILCE-7M3", 9); // includes the NUL terminator
    const quint32 ifd0 = 8, ifd0Size = 2 + 3 * 12 + 4;
    const quint32 modelOff = ifd0 + ifd0Size;
    const quint32 exifIfd = modelOff + quint32(model.size()) + 1; // keep it even
    const quint32 exifSize = 2 + 4 * 12 + 4;
    const quint32 rationals = exifIfd + exifSize;

    tiff.append("II*\0", 4);
    u32(ifd0);
    u16(3);
    entry(0x0110, 2, quint32(model.size()), modelOff); // Model
    entry(0x0112, 3, 1, orientation);                  // Orientation (SHORT, left-aligned)
    entry(0x8769, 4, 1, exifIfd);                      // ExifIFD pointer
    u32(0);
    tiff.append(model).append('\0');

    u16(4);
    entry(0x829A, 5, 1, rationals);      // ExposureTime
    entry(0x829D, 5, 1, rationals + 8);  // FNumber
    entry(0x8827, 3, 1, 400);            // ISO
    entry(0x920A, 5, 1, rationals + 16); // FocalLength
    u32(0);
    u32(1); u32(250);
    u32(28); u32(10);
    u32(35); u32(1);
    return tiff;
}

QByteArray withExif(const QByteArray &jpeg, quint16 orientation)
{
    const QByteArray payload = "Exif\0\0"_ba + exifTiff(orientation);
    QByteArray segment("\xFF\xE1", 2);
    const quint16 length = quint16(payload.size() + 2);
    segment.append(char(length >> 8)).append(char(length & 0xFF)).append(payload);
    return jpeg.left(2) + segment + jpeg.mid(2);
}

void writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(data);
}

QSize imageSize(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(false);
    return reader.size();
}

} // namespace

class TestEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        const int id = QFontDatabase::addApplicationFont(ASSETS_DIR "/Go-Regular.ttf"_L1);
        QVERIFY(id >= 0);
        Watermark::setFontFamily(QFontDatabase::applicationFontFamilies(id).value(0));
    }

    // ── isSupported / outputPath ───────────────────────────────────────────────
    void isSupported_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<bool>("expected");
        QTest::newRow("jpg") << u"a/photo.jpg"_s << true;
        QTest::newRow("JPEG upper") << u"photo.JPEG"_s << true;
        QTest::newRow("png") << u"photo.png"_s << true;
        QTest::newRow("ARW") << u"DSC0001.ARW"_s << true;
#ifdef HAVE_JXL
        QTest::newRow("jxl") << u"photo.jxl"_s << true;
#endif
        QTest::newRow("txt") << u"notes.txt"_s << false;
        QTest::newRow("heic") << u"photo.heic"_s << false;
        QTest::newRow("no extension") << u"photo"_s << false;
    }
    void isSupported()
    {
        QFETCH(QString, path);
        QFETCH(bool, expected);
        QCOMPARE(Watermark::isSupported(path), expected);
    }

    void supportedFormats()
    {
        const QStringList formats = Watermark::supportedFormats();
        for (const QString &f : { u"ARW"_s, u"JPG"_s, u"PNG"_s })
            QVERIFY2(formats.contains(f), qPrintable(f));
#ifdef HAVE_JXL
        QVERIFY(formats.contains(u"JXL"_s));
#endif
    }

    void outputPath_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expected");
        QTest::newRow("raw") << u"/photos/DSC0001.ARW"_s << u"/out/DSC0001.jpg"_s;
        QTest::newRow("png") << u"/photos/shot.png"_s << u"/out/shot.jpg"_s;
        QTest::newRow("dots in name") << u"/photos/my.trip.jpeg"_s << u"/out/my.trip.jpg"_s;
    }
    void outputPath()
    {
        QFETCH(QString, input);
        QFETCH(QString, expected);
        QCOMPARE(Watermark::outputPath(input, u"/out"_s), expected);
    }

    // ── collectFiles ───────────────────────────────────────────────────────────
    void collectFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString root = dir.path();
        QVERIFY(QDir(root).mkpath(u"sub"_s));
        QVERIFY(QDir(root).mkpath(u"out"_s));
        for (const QString &name : { u"img10.jpg"_s, u"img2.JPG"_s, u"notes.txt"_s,
                                     u"sub/nested.png"_s, u"out/done.jpg"_s })
            writeFile(root + u'/' + name, "x");
        const QString out = root + u"/out"_s;

        QCOMPARE(Watermark::collectFiles({ root }, false, out),
                 QStringList({ root + u"/img2.JPG"_s, root + u"/img10.jpg"_s }));
        QCOMPARE(Watermark::collectFiles({ root }, true, out),
                 QStringList({ root + u"/img2.JPG"_s, root + u"/img10.jpg"_s,
                               root + u"/sub/nested.png"_s }));
        // Direct files are taken as-is (once), unsupported ones dropped.
        QCOMPARE(Watermark::collectFiles({ root + u"/img10.jpg"_s, root + u"/img10.jpg"_s,
                                           root + u"/notes.txt"_s }, false, out),
                 QStringList({ root + u"/img10.jpg"_s }));
        QVERIFY(Watermark::collectFiles({ root + u"/missing"_s }, true, out).isEmpty());

        QTemporaryDir empty;
        QVERIFY(Watermark::collectFiles({ empty.path() }, true, out).isEmpty());
    }

    // ── resizeForWhatsApp ──────────────────────────────────────────────────────
    void resizeForWhatsApp_data()
    {
        QTest::addColumn<QSize>("input");
        QTest::addColumn<QSize>("expected");
        QTest::newRow("small unchanged") << QSize(800, 600) << QSize(800, 600);
        QTest::newRow("exact limit unchanged") << QSize(2560, 1440) << QSize(2560, 1440);
        QTest::newRow("landscape") << QSize(5120, 2560) << QSize(2560, 1280);
        QTest::newRow("portrait") << QSize(3000, 6000) << QSize(1280, 2560);
    }
    void resizeForWhatsApp()
    {
        QFETCH(QSize, input);
        QFETCH(QSize, expected);
        QCOMPARE(Watermark::resizeForWhatsApp(solidImage(input.width(), input.height())).size(),
                 expected);
    }

    // ── applyOrientation ───────────────────────────────────────────────────────
    void applyOrientation_data()
    {
        // A 3×2 image with a red top-left pixel: where does that pixel end up?
        QTest::addColumn<int>("orientation");
        QTest::addColumn<QSize>("size");
        QTest::addColumn<QPoint>("red");
        QTest::newRow("1 normal") << 1 << QSize(3, 2) << QPoint(0, 0);
        QTest::newRow("3 rotate 180") << 3 << QSize(3, 2) << QPoint(2, 1);
        QTest::newRow("6 rotate 90 cw") << 6 << QSize(2, 3) << QPoint(1, 0);
        QTest::newRow("8 rotate 270 cw") << 8 << QSize(2, 3) << QPoint(0, 2);
        QTest::newRow("unknown") << 42 << QSize(3, 2) << QPoint(0, 0);
    }
    void applyOrientation()
    {
        QFETCH(int, orientation);
        QFETCH(QSize, size);
        QFETCH(QPoint, red);
        QImage img = solidImage(3, 2, Qt::black);
        img.setPixelColor(0, 0, Qt::red);
        const QImage out = Watermark::applyOrientation(img, orientation);
        QCOMPARE(out.size(), size);
        QCOMPARE(out.pixelColor(red), QColor(Qt::red));
    }

    // ── readExif ───────────────────────────────────────────────────────────────
    void readExif_empty()
    {
        const PhotoSettings s = readExif({});
        QVERIFY(s.aperture.isEmpty() && s.cameraModel.isEmpty());
        QCOMPARE(s.orientation, 1);
    }

    void readExif_jpegWithoutExif()
    {
        const PhotoSettings s = readExif(encodeJpeg(solidImage(16, 16)));
        QVERIFY(s.iso.isEmpty() && s.shutterSpeed.isEmpty());
        QCOMPARE(s.orientation, 1);
    }

    void readExif_jpegWithExif()
    {
        const PhotoSettings s = readExif(withExif(encodeJpeg(solidImage(16, 16)), 6));
        QCOMPARE(s.cameraModel, u"ILCE-7M3"_s);
        QCOMPARE(s.aperture, u"2.8"_s);
        QCOMPARE(s.shutterSpeed, u"1/250"_s);
        QCOMPARE(s.iso, u"400"_s);
        QCOMPARE(s.focalLength, u"35mm"_s);
        QCOMPARE(s.orientation, 6);
    }

    // ── addWatermark ───────────────────────────────────────────────────────────
    void addWatermark_emptySettingsUnchanged()
    {
        const QImage img = solidImage(200, 150);
        QCOMPARE(Watermark::addWatermark(img, {}, 0.82), img);
    }

    void addWatermark_drawsBottomCentre()
    {
        const QImage img = solidImage(800, 600, Qt::white);
        PhotoSettings s;
        s.aperture = u"2.8"_s;
        s.shutterSpeed = u"1/250"_s;
        s.iso = u"400"_s;
        s.focalLength = u"50mm"_s;
        s.cameraModel = u"Test Camera"_s;
        const QImage out = Watermark::addWatermark(img, s, 0.82);
        QCOMPARE(out.size(), img.size());
        QCOMPARE(out.pixelColor(5, 5), QColor(Qt::white));
        bool changed = false;
        for (int y = 450; y < 600 && !changed; ++y)
            changed = out.pixelColor(400, y) != QColor(Qt::white);
        QVERIFY2(changed, "expected the watermark pill near the bottom centre");
    }

    void addWatermark_zeroOpacity()
    {
        PhotoSettings s;
        s.iso = u"200"_s;
        QCOMPARE(Watermark::addWatermark(solidImage(400, 300), s, 0.0).size(), QSize(400, 300));
    }

    // ── decodeImage ────────────────────────────────────────────────────────────
    void decodeImage_validJpeg()
    {
        QString error;
        const QImage img = Watermark::decodeImage(u"a.jpg"_s, encodeJpeg(solidImage(64, 32)), &error);
        QCOMPARE(img.size(), QSize(64, 32));
        QVERIFY(error.isEmpty());
    }

    void decodeImage_garbage()
    {
        QString error;
        QVERIFY(Watermark::decodeImage(u"a.jpg"_s, "not an image", &error).isNull());
        QVERIFY(!error.isEmpty());
    }

    void decodeImage_arwPicksLargestEmbeddedJpeg()
    {
        const QByteArray raw = "TIFFHEADER"_ba + encodeJpeg(solidImage(32, 24)) + "RAWDATA"_ba
                             + encodeJpeg(solidImage(320, 240)) + "TRAILER"_ba;
        QString error;
        QCOMPARE(Watermark::decodeImage(u"DSC0001.ARW"_s, raw, &error).size(), QSize(320, 240));
    }

    // ── processFile ────────────────────────────────────────────────────────────
    void processFile_resizesRotatesAndWritesJpeg()
    {
        QTemporaryDir dir;
        const QString input = dir.filePath(u"DSC0001.jpg"_s);
        writeFile(input, withExif(encodeJpeg(solidImage(3000, 2000)), 6));
        const QString out = dir.filePath(u"out"_s);
        QVERIFY(QDir().mkpath(out));

        QCOMPARE(Watermark::processFile(input, out, 0.82), QString());
        // 3000×2000 → resized to 2560×1707, then rotated by orientation 6.
        QCOMPARE(imageSize(out + u"/DSC0001.jpg"_s), QSize(1707, 2560));
    }

    void processFile_missingInput()
    {
        QTemporaryDir dir;
        const QString error = Watermark::processFile(dir.filePath(u"nope.jpg"_s), dir.path(), 0.82);
        QVERIFY(error.startsWith(u"open: "_s));
    }

#ifdef HAVE_JXL
    // ── JPEG XL ────────────────────────────────────────────────────────────────
    void jxl_readsCompressedExifAndOrientation()
    {
        QFile file(TEST_DATA_DIR "/rotated_exif.jxl"_L1);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QByteArray exif;
        QString error;
        const QImage img = decodeJxl(file.readAll(), &exif, &error);
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QCOMPARE(img.size(), QSize(48, 64)); // 64×48 stored, orientation 6 applied
        const PhotoSettings s = readExif(exif);
        QCOMPARE(s.cameraModel, u"ILCE-7M3"_s);
        QCOMPARE(s.aperture, u"2.8"_s);
        QCOMPARE(s.iso, u"400"_s);
    }

    void jxl_processFile()
    {
        QTemporaryDir dir;
        QVERIFY(QFile::copy(TEST_DATA_DIR "/rotated_exif.jxl"_L1, dir.filePath(u"photo.jxl"_s)));
        QCOMPARE(Watermark::processFile(dir.filePath(u"photo.jxl"_s), dir.path(), 0.82), QString());
        QCOMPARE(imageSize(dir.filePath(u"photo.jpg"_s)), QSize(48, 64));
    }

    void jxl_brokenFile()
    {
        QString error;
        QVERIFY(decodeJxl("\xFF\x0A garbage"_ba, nullptr, &error).isNull());
        QVERIFY(error.startsWith(u"JPEG XL"_s));
    }
#endif
};

QTEST_MAIN(TestEngine)
#include "tst_engine.moc"
