#pragma once

#include <QImage>
#include <QString>

#include "exif.h"

namespace Watermark {

inline constexpr int MaxWhatsAppDim = 2560;
inline constexpr int JpegQuality = 92;

// Font family used to draw the watermark text (registered at startup).
void setFontFamily(const QString &family);

// Input formats this build can read, as upper-case extensions ("ARW", "JPG", …).
QStringList supportedFormats();
bool isSupported(const QString &path);
QString outputPath(const QString &inputPath, const QString &outDir);

QImage decodeImage(const QString &path, const QByteArray &data, QString *error);
QImage applyOrientation(const QImage &image, int orientation);
QImage resizeForWhatsApp(const QImage &image);
QImage addWatermark(const QImage &image, const PhotoSettings &settings, double opacity);

// Full pipeline for one file. Returns an empty string on success, otherwise the error.
// Safe to call from worker threads.
QString processFile(const QString &inputPath, const QString &outDir, double opacity);

} // namespace Watermark
