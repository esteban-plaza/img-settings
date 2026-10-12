#pragma once

#include <QByteArray>
#include <QImage>
#include <QString>

// Decodes a JPEG XL file to 8-bit sRGB. The EXIF orientation is already applied
// by libjxl. If `exif` is given it receives the TIFF bytes of the Exif box (if any).
QImage decodeJxl(const QByteArray &data, QByteArray *exif, QString *error);
