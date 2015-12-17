// -*- C++ -*-
#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include "src/image.h"
#include "src/image_reader.h"
#include "src/aligned_storage.h"

bool is_png(const std::string &src);

// Forward declare of values in png_info...
struct png_struct_def;
struct png_info_struct;

class png_reader final : public image_reader {
    unsigned int width_ = 0, height_ = 0;
    png_struct_def *png_ = nullptr;
    png_info_struct *info_ = nullptr;
    // TODO: stop using a stringbuf...
    std::stringbuf read_buf_;

    Aligned<ARGBPixel> src_line_buf_;
    XRGBImage tmp_img_;

    ARGBPixel *src_line_buf();

    Image decode_impl() final;

    void junk_lines() noexcept;
    void read_lines() noexcept;

   public:
    img_type file_type() const final { return PNG; }

    unsigned int src_height() const final { return ok() ? height_ : 0; }
    unsigned int src_width() const final { return ok() ? width_ : 0; }

    explicit png_reader(const std::string &src);
    ~png_reader();
};

std::string encode_png(const XRGBImage &src);
