// -*- C++ -*-
#pragma once
#include "image.h"
#include "bounded_int.h"

namespace config {
struct resize {
#define ENTRY(_section, _name, _type, _default, _docstring)                    \
  _type _name = _type(_default);
#include "resize_config-x.def"
#undef ENTRY
}; // sruct resize
} // namespace config

enum class scale_method {
  FASTEST, // Point or fast bilinear
  NORMAL,  // Bilinear/Box.
  BEST     // lanczos (or bilinear if no lanczos).
};

Image resize(const Image &img, unsigned int w, unsigned int h,
             scale_method mthd);
