// -*- C++ -*-
#pragma once
#include <string>
#include <csetjmp>
extern "C" {
#include "externals/libjpeg-turbo/jpeglib.h"
}
#include "src/image.h"
#include "src/image_reader.h"
#include "src/bounded_int.h"
#include "src/convert.h"
#include "src/config.h"

bool is_jpeg(const std::string &src);

class jpeg_reader final : public image_reader, public jpeg_error_mgr {
   private:
    jmp_buf jmp_buffer_;
    struct jpeg_decompress_struct dinfo_;
    static void fatal_error(j_common_ptr cinfo) __attribute__((noreturn));
    static void non_fatal_error(j_common_ptr cinfo);

    Image decode_impl() override;

    Image fancy_decode();

    template <typename _IMG>
    Image planar_decode();

    unsigned int set_scale(unsigned int);

   public:
    explicit jpeg_reader(const std::string &src,
                         const config::jpeg &config = config::jpeg());

    img_type file_type() const override { return JPEG; }

    unsigned int src_width() const override {
        return ok() ? dinfo_.image_width : 0;
    }
    unsigned int src_height() const override {
        return ok() ? dinfo_.image_height : 0;
    }

    ~jpeg_reader();
};

std::string encode_jpeg(const Image &img,
                        const config::jpeg &config = config::jpeg());
