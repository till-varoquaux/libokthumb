#include "image_reader.h"
#include <cassert>
#include <algorithm>
#include "aligned_storage.h"
#include "jpeg_codec.h"
#include "gif_codec.h"
#include "png_codec.h"
#include "logging.h"
#include "config.h"

void image_reader::set_scale(unsigned int) {}

unsigned int image_reader::get_scale() const { return 1; }

image_reader::~image_reader() {}

void image_reader::set_error(std::string &&msg) { error_ = std::move(msg); }

void image_reader::set_warning(std::string &&msg) { warning_ = std::move(msg); }

unsigned int image_reader::scaled_width() const {
    return up_div(src_width(), get_scale());
}

unsigned int image_reader::scaled_height() const {
    return up_div(src_height(), get_scale());
}

Image image_reader::decode() {
    if (!ok()) {
        return Image();
    }

    if (dims.right_x == 0) dims.right_x = src_width();
    if (dims.bottom_y == 0) dims.bottom_y = src_height();

    if (dims.right_x > src_width() || dims.right_x <= dims.left_x ||
        dims.bottom_y > src_height() || dims.bottom_y <= dims.top_y) {
        INFO_LOGGER << "Crop square error info:\n"
                    << "scale: " << get_scale()
                    << "\nsrc_width: " << src_width()
                    << "\nleft-x:" << dims.left_x << " ,top-y:" << dims.top_y
                    << "\nright-x:" << dims.right_x
                    << " ,bottom_y:" << dims.bottom_y << std::endl;
        set_error("While reading image: Invalid crop square requested....");
        return Image();
    }

    unsigned int desired_scale = std::min(dims.width() / dims.dst_width,
                                          dims.height() / dims.dst_height);

    if (desired_scale > 1) {
        set_scale(desired_scale);
    }

    unsigned int scale = get_scale();
    dims.left_x = dims.left_x / scale;
    dims.top_y = dims.top_y / scale;
    dims.right_x = dims.right_x / scale;
    dims.bottom_y = dims.bottom_y / scale;

    return decode_impl(dims);
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
