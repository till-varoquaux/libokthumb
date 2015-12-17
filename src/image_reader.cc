#include "src/image_reader.h"
#include <cassert>
#include <algorithm>
#include "src/aligned_storage.h"
#include "src/jpeg_codec.h"
#include "src/gif_codec.h"
#include "src/png_codec.h"
#include "src/logging.h"
#include "src/config.h"

image_reader::~image_reader() {}

void image_reader::set_error(std::string &&msg) { error_ = std::move(msg); }

void image_reader::set_warning(std::string &&msg) { warning_ = std::move(msg); }

Image image_reader::decode() {
    if (!ok()) {
        return Image();
    }

    if (dims.right_x == 0) dims.right_x = src_width();
    if (dims.bottom_y == 0) dims.bottom_y = src_height();

    if (dims.right_x > src_width() || dims.right_x <= dims.left_x ||
        dims.bottom_y > src_height() || dims.bottom_y <= dims.top_y) {
        INFO_LOGGER << "Crop square error info:\n"
                    << "\nsrc_width: " << src_width()
                    << "\nleft-x:" << dims.left_x << " ,top-y:" << dims.top_y
                    << "\nright-x:" << dims.right_x
                    << " ,bottom_y:" << dims.bottom_y << std::endl;
        set_error("While reading image: Invalid crop square requested....");
        return Image();
    }

    return decode_impl();
}

img::file_type img::get_mime_type(const std::string &data) {
    if (is_jpeg(data)) {
        return img::file_type::JPEG;
    } else if (is_png(data)) {
        return img::file_type::PNG;
    } else if (is_gif(data)) {
        return img::file_type::GIF;
    } else {
        return img::file_type::FAILED;
    }
}

std::unique_ptr<image_reader> img::get_reader(const std::string &data) {
    return get_reader(data, config::jpeg());
}

// Tries to guess the image type using the magic number and then
std::unique_ptr<image_reader> img::get_reader(const std::string &data,
                                              const config::jpeg &jconfig) {
    typedef std::unique_ptr<image_reader> ret;
    if (is_jpeg(data)) {
        return ret(new (std::nothrow) jpeg_reader(data, jconfig));
    } else if (is_png(data)) {
        return ret(new (std::nothrow) png_reader(data));
    } else if (is_gif(data)) {
        return ret(new (std::nothrow) gif_reader(data));
    }
    return ret(nullptr);
}
