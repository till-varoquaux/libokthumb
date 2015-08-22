// -*- C++ -*-
#pragma once
#include <memory>
#include <string>
struct Config;

// This is the high level interface to the library. We shall try to keep api and
// abi compatibility

enum class file_type {
    PPM,            // For testing purposes... Seriously slow and non-fancy
    JPEG,
    WEBP            // for Chromium
    //    BPG       // http://bellard.org/bpg/
    //    JPEG2000  // for safari
};

class image_reader;

struct ImgPipeline {
    const std::string &error() const;
    const std::string &warning() const;

    ImgPipeline(const std::string&, const Config&);
    // TODO: take a const char* and a length

    unsigned int src_height() const;
    unsigned int src_width() const;

    // The crop happens before the resize
    void crop(unsigned int left_x = 0,
              unsigned int top_y = 0,
              unsigned int right_x = 0,  // 0 means right edge
              unsigned int bottom_y = 0); // 0 means bottom edge

    void resize(unsigned int width, unsigned int height);

    // Run should take a dst type with a `virtual void set_size(unsigned)` and
    // `const char* data()` and write to it
    std::string run(file_type);

protected:
    std::unique_ptr<image_reader> reader_;
    const Config* config_;
};

