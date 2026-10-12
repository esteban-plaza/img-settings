#pragma once

#include <QByteArray>
#include <QString>

// Camera settings read from EXIF, already formatted for display.
struct PhotoSettings {
    QString aperture;      // "2.8"
    QString shutterSpeed;  // "1/250" or "2s"
    QString iso;           // "400"
    QString focalLength;   // "35mm"
    QString cameraModel;   // "ILCE-7M3"
    int orientation = 1;   // EXIF orientation, 1..8
};

// Reads EXIF from the raw bytes of a JPEG, PNG (eXIf chunk) or TIFF-based raw (ARW).
// Missing fields are left empty; never fails.
PhotoSettings readExif(const QByteArray &data);
