// -*- C++ -*-
#pragma once
#include <memory>
#include <string>
#include "src/image.h"

namespace config {
struct jpeg;
}  // namespace config

class image_reader {
    std::string error_, warning_;

    friend struct ImgPipeline;

   protected:
    struct dim_t {
        unsigned int dst_width = 0, left_x = 0, top_y = 0,
                     right_x = 0,  // 0 means right edge
                bottom_y = 0;      // 0 means bottom edge
        ColorSpace desired_color_space = ColorSpace::YUV420;
        unsigned int width() const { return right_x - left_x; }
        unsigned int height() const { return bottom_y - top_y; }
    } dims;

    void set_error(std::string &&);
    void set_warning(std::string &&);

    virtual Image decode_impl() = 0;
   public:
    enum img_type { JPEG, GIF, PNG };

    bool ok() const { return error_.length() == 0; }

    const std::string &error() const { return error_; }

    const std::string &warning() const { return warning_; }

    virtual unsigned int src_width() const = 0;

    virtual unsigned int src_height() const = 0;

    void set_desired_width(unsigned int desired) {
        dims.dst_width = desired;
    }

    image_reader() = default;
    explicit image_reader(image_reader &) = delete;
    image_reader &operator=(image_reader &) = delete;

    virtual img_type file_type() const = 0;

    Image decode();

    virtual ~image_reader();
};

// inline image_reader::~image_reader(){}

namespace img {

enum class file_type { FAILED, JPEG, GIF, PNG };

file_type get_mime_type(const std::string &data);

std::unique_ptr<image_reader> get_reader(const std::string &data,
                                         const config::jpeg &jconfig);

std::unique_ptr<image_reader> get_reader(const std::string &data);
}  // namespace img
