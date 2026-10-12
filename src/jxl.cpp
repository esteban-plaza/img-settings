#include "jxl.h"

#include <jxl/cms.h>
#include <jxl/color_encoding.h>
#include <jxl/decode_cxx.h>

#include <cstring>

using namespace Qt::StringLiterals;

namespace {

// The Exif box starts with a 4-byte big-endian offset to the TIFF header.
QByteArray tiffFromExifBox(const QByteArray &box)
{
    if (box.size() < 4)
        return {};
    const quint32 offset = quint32(quint8(box[0])) << 24 | quint8(box[1]) << 16
                         | quint8(box[2]) << 8 | quint8(box[3]);
    if (qsizetype(offset) > box.size() - 4)
        return {};
    return box.mid(4 + offset);
}

} // namespace

QImage decodeJxl(const QByteArray &data, QByteArray *exif, QString *error)
{
    auto fail = [error](const QString &why) {
        if (error)
            *error = u"JPEG XL: "_s + why;
        return QImage();
    };

    auto dec = JxlDecoderMake(nullptr);
    int events = JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE;
    if (exif)
        events |= JXL_DEC_BOX;
    if (JxlDecoderSubscribeEvents(dec.get(), events) != JXL_DEC_SUCCESS)
        return fail(u"decoder setup failed"_s);

    // Lets the decoder convert any source color space to sRGB.
    JxlDecoderSetCms(dec.get(), *JxlGetDefaultCms());
    JxlDecoderSetDecompressBoxes(dec.get(), JXL_TRUE); // Exif may be Brotli-compressed ("brob")
    JxlDecoderSetInput(dec.get(), reinterpret_cast<const uint8_t *>(data.constData()), size_t(data.size()));
    JxlDecoderCloseInput(dec.get());

    JxlBasicInfo info {};
    const JxlPixelFormat format { 4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0 };
    QImage image;
    QByteArray box;
    bool readingExif = false;

    auto finishExifBox = [&] {
        const size_t unused = JxlDecoderReleaseBoxBuffer(dec.get());
        box.resize(box.size() - qsizetype(unused));
        if (exif->isEmpty())
            *exif = tiffFromExifBox(box);
        readingExif = false;
    };

    for (;;) {
        switch (JxlDecoderProcessInput(dec.get())) {
        case JXL_DEC_ERROR:
            return fail(u"the file is damaged or unsupported"_s);
        case JXL_DEC_NEED_MORE_INPUT:
            return fail(u"the file is truncated"_s);

        case JXL_DEC_BASIC_INFO:
            if (JxlDecoderGetBasicInfo(dec.get(), &info) != JXL_DEC_SUCCESS)
                return fail(u"unreadable header"_s);
            break;

        case JXL_DEC_COLOR_ENCODING: {
            // Output is an 8-bit JPEG, so ask for sRGB (handles wide-gamut and HDR sources).
            JxlColorEncoding srgb {};
            srgb.color_space = JXL_COLOR_SPACE_RGB;
            srgb.white_point = JXL_WHITE_POINT_D65;
            srgb.primaries = JXL_PRIMARIES_SRGB;
            srgb.transfer_function = JXL_TRANSFER_FUNCTION_SRGB;
            srgb.rendering_intent = JXL_RENDERING_INTENT_PERCEPTUAL;
            JxlDecoderSetOutputColorProfile(dec.get(), &srgb, nullptr, 0);
            break;
        }

        case JXL_DEC_NEED_IMAGE_OUT_BUFFER:
            image = QImage(int(info.xsize), int(info.ysize), QImage::Format_RGBA8888);
            if (image.isNull())
                return fail(u"image too large"_s);
            if (JxlDecoderSetImageOutBuffer(dec.get(), &format, image.bits(), size_t(image.sizeInBytes()))
                != JXL_DEC_SUCCESS)
                return fail(u"unsupported pixel layout"_s);
            break;

        case JXL_DEC_FULL_IMAGE:
            // Keep going: the Exif box may come after the codestream.
            break;

        case JXL_DEC_BOX: {
            if (readingExif)
                finishExifBox();
            JxlBoxType type;
            if (JxlDecoderGetBoxType(dec.get(), type, JXL_TRUE) == JXL_DEC_SUCCESS
                && std::memcmp(type, "Exif", 4) == 0) {
                readingExif = true;
                box.resize(64 * 1024);
                JxlDecoderSetBoxBuffer(dec.get(), reinterpret_cast<uint8_t *>(box.data()), size_t(box.size()));
            }
            break;
        }

        case JXL_DEC_BOX_NEED_MORE_OUTPUT: {
            const size_t unused = JxlDecoderReleaseBoxBuffer(dec.get());
            const qsizetype used = box.size() - qsizetype(unused);
            box.resize(box.size() * 2);
            JxlDecoderSetBoxBuffer(dec.get(), reinterpret_cast<uint8_t *>(box.data()) + used,
                                   size_t(box.size() - used));
            break;
        }

        case JXL_DEC_SUCCESS:
            if (readingExif)
                finishExifBox();
            return image.isNull() ? fail(u"no image data"_s) : image;

        default:
            break;
        }
    }
}
