// -*- C++ -*-
#pragma once
#include "image.h"
#include "image_reader.h"
#include <string>
#include <csetjmp>
#include "bounded_int.h"

extern "C" {
#include <externals/libjpeg-turbo/jpeglib.h>
}

namespace config {
struct jpeg {
#define ENTRY(_section, _name, _type, _default, _docstring)                    \
  _type _name = _type(_default);
#include "jpeg_config-x.def"
#undef ENTRY
}; // sruct jpeg
} // namespace config

bool is_jpeg(const std::string &src);

class jpeg_reader final : public image_reader, public jpeg_error_mgr {
private:
  jmp_buf jmp_buffer_;
  struct jpeg_decompress_struct dinfo_;
  static void fatal_error(j_common_ptr cinfo) __attribute__((noreturn));
  static void non_fatal_error(j_common_ptr cinfo);

  Image decode_impl(dim_t dims) override;

  Image fancy_decode(dim_t dims);

  template <typename _IMG> Image planar_decode(const dim_t img_dims);

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

  unsigned int scaled_width() const override {
    return ok() ? dinfo_.output_width : 0;
  }

  unsigned int scaled_height() const override {
    return ok() ? dinfo_.output_height : 0;
  }

  void set_scale(unsigned int) override;
  unsigned int get_scale() const override;

  ~jpeg_reader();
};

std::string encode_jpeg(const Image &img,
                        const config::jpeg &config = config::jpeg());
