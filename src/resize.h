// -*- C++ -*-
#pragma once
#include "image.h"
#include "bounded_int.h"
#include "config.h"

enum class scale_method {
    FASTEST,  // Point or fast bilinear
    NORMAL,   // Bilinear/Box.
    BEST      // lanczos (or bilinear if no lanczos).
};

Image resize(const Image &img, unsigned int w, unsigned int h,
             scale_method mthd);
