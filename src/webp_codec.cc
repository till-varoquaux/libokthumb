#include "webp_codec.h"
#include <string>
#include <cstring>
#include <memory>
#include "logging.h"
#include "convert.h"
#include <externals/libwebp/src/webp/encode.h>

namespace {
int write_to_string(const uint8_t *data, size_t data_size,
                    const WebPPicture *picture) {
    std::string *tgt = static_cast<std::string *>(picture->custom_ptr);
    tgt->append(reinterpret_cast<const char *>(data), data_size);
    return 1;
}

std::string encode(const Yuv420Image &img, unsigned char quality) {
    WebPPicture picture;
    WebPConfig config;
    std::string res;

    if (!WebPPictureInit(&picture)) {
        ERR_LOGGER << "Webp internal error" << std::endl;
        return std::string();
    }

    picture.width = static_cast<int>(img.width());
    picture.height = static_cast<int>(img.height());

    picture.y = const_cast<uint8_t *>(img.data<0>());
    picture.u = const_cast<uint8_t *>(img.data<1>());
    picture.v = const_cast<uint8_t *>(img.data<2>());
    picture.y_stride = static_cast<int>(img.stride<0>());
    picture.uv_stride = static_cast<int>(img.stride<1>());
    picture.writer = write_to_string;
    picture.custom_ptr = &res;

    if (!WebPConfigPreset(&config, WEBP_PRESET_PICTURE, quality)) {
        ERR_LOGGER << "Webp internal error" << std::endl;
        return std::string();
    }

    config.lossless = 0;

    if (!WebPValidateConfig(&config)) {
        ERR_LOGGER << "Error! Invalid configuration." << std::endl;
        return std::string();
    }

    if (!WebPEncode(&config, &picture)) {
        ERR_LOGGER << "Webp internal error: " << picture.error_code
                   << std::endl;
        return std::string();
    }

    return res;
}

}  // namespace

std::string encode_webp(const Image &img, unsigned char quality) {
    return encode(*to_yuv420(img), quality);
}
