// -*- C++ -*-
#pragma once
#include <sstream>
#include <string>
#include "src/image.h"
#include "src/image_reader.h"

bool is_gif(const std::string &src);

struct GifFileType;

class gif_reader final : public image_reader {
    std::stringbuf sbuf;
    GifFileType *gif;

    Image decode_impl() final;

   public:
    unsigned int src_width() const final;
    unsigned int src_height() const final;

    explicit gif_reader(const std::string &src);
    img_type file_type() const final { return GIF; }
    ~gif_reader();
};
