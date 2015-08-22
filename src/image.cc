#include "image.h"
#include <cassert>
#include <algorithm>
#include "aligned_storage.h"
#include <include/libyuv.h>

extern "C" {
#include <endian.h>
}

static_assert(sizeof(ARGBPixel) == sizeof(int32_t), "pixel packing error");


const char *to_string(ColorSpace ty) {
  switch (ty) {
  case ColorSpace::YUV420:
    return "yuv420";
  case ColorSpace::YUV444:
    return "yuv444";
  case ColorSpace::XRGB:
    return "rgbx";
  case ColorSpace::GRAYSCALE:
    return "grayscale";
  }
}


//------------------------------------------------------------------------------

// template <typename _SRC, typename _DST, // typename _F, _F *f,
//           size_t... _indices_src, size_t... _indices_dst>
// static _SRC convert(const _SRC &src) {
//     _SRC res(src.width(), src.height());
//     libyuv::ARGBToI400(src.template data<_indices_src>(),
//                        static_cast<int>(src.template  stride<_indices_src>())...,
//                        res.template data<_indices_dst>(),
//                        static_cast<int>(res.template  stride<_indices_dst>())...,
//                        static_cast<int>(src.width()),
//                        static_cast<int>(src.height()));
//     return res;
// }

