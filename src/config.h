// -*- C++ -*-
#pragma once
#include "src/bounded_int.h"

namespace config {
#define ENTRY(_section, _name, _type, _default, _docstring) \
    _type _name = _type(_default);

struct resize {
#include "src/resize_config-x.def"
};  // struct resize

struct jpeg {
#include "src/jpeg_config-x.def"
};  // struct jpeg

#undef ENTRY
}  // namespace config

struct Config {
    config::jpeg jpeg;
    config::resize resize;
};
