// -*- C++ -*-
#pragma once
#include <string>
#include "src/image.h"
std::string encode_webp(const Image &img, unsigned char quality = 75);
