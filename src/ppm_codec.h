// -*- C++ -*-
#pragma once
#include <string>
#include "src/image.h"

// This codec is intended to be used for debug purposes only. It is slow and
// incomplete but damn simple.
std::string encode_ppm(const XRGBImage &img);

std::string encode_ppm(const Image &img);
