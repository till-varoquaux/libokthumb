// -*- C++ -*-
#pragma once
#include "src/image.h"
#include "src/bounded_int.h"
#include "src/config.h"

enum class scale_method {
    FASTEST,  // Point or fast bilinear
    NORMAL,   // Bilinear/Box.
    BEST      // lanczos (or bilinear if no lanczos).
};

Image resize(const Image &img, unsigned int w, unsigned int h,
             scale_method mthd);
