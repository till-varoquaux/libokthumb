#include "image.h"
#include <cassert>
#include <algorithm>
#include "aligned_storage.h"
#include <include/libyuv.h>

extern "C" {
#include <endian.h>
}

static_assert(sizeof(ARGBPixel) == sizeof(int32_t), "pixel packing error");

// // This is a slow and jenky conversion
// // http://en.wikipedia.org/wiki/YCbCr
// -- To yuv
//   int yv =static_cast<int>
//     (0.2569 * src.r + 0.5044 * src.g + 0.0979 * src.b + 16);
//   uint8_t y = static_cast<uint8_t> (yv>235? 235 : yv<16 ? 16 : yv);
//   int uv =(int)(-0.1483 * src.r - 0.2911 * src.g + 0.4394 * src.b + 128);
//   uint8_t u = static_cast<uint8_t>(uv>240? 240 : uv<16 ? 16 : uv);
//   int vv = (int)(0.4394 * src.r - 0.3679 * src.g - 0.0715 * src.b + 128);
//   uint8_t v = static_cast<uint8_t>(vv>240? 240 : vv<16 ? 16 : vv);
//

// y to rgb-> 1.164 * (y-16)

// -- To rgb
//   int r, g, b;
//   r = (int)(1.164 * (*y-16) + 1.596 * (*v-128));
//   g = (int)(1.164 * (*y-16) - 0.813 * (*v-128) - 0.391 * (*u-128));
//   b = (int)(1.164 * (*y-16)                   + 2.018 * (*u-128));

//   return {
//       static_cast<unsigned char> (r>255? 255 : r<0 ? 0 : r),
//       static_cast<unsigned char> (g>255? 255 : g<0 ? 0 : g),
//       static_cast<unsigned char> (b>255? 255 : b<0 ? 0 : b),
//       255
//  };
// }

// To grayscale:
//   Y = (6969 * R + 23434 * G + 2365 * B)/32768

const char *to_string(ImageType ty) {
  switch (ty) {
  case ImageType::YUV420:
    return "yuv420";
  case ImageType::YUV444:
    return "yuv444";
  case ImageType::XRGB:
    return "rgbx";
  case ImageType::GRAYSCALE:
    return "grayscale";
  }
}

Yuv420Image to_yuv420(const GrayscaleImage &gs) {
  Yuv420Image res(gs.width(), gs.height());
  assert(gs.size<0>() == res.size<0>());
  memcpy(res.data<0>(), gs.data<0>(), res.size<0>());
  memset(res.data<1>(), 128, res.size<1>());
  memset(res.data<2>(), 128, res.size<2>());
  return res;
}

GrayscaleImage to_grayscale(const Yuv420Image &yuv) {
  GrayscaleImage res(yuv.width(), yuv.height());
  assert(yuv.size<0>() == res.size<0>());
  memcpy(res.data<0>(), yuv.data<0>(), res.size<0>());
  return res;
}

GrayscaleImage to_grayscale(const Yuv444Image &yuv) {
  GrayscaleImage res(yuv.width(), yuv.height());
  assert(yuv.size<0>() == res.size<0>());
  memcpy(res.data<0>(), yuv.data<0>(), res.size<0>());
  return res;
}

Yuv420Image to_yuv420(const XRGBImage &rgbx) {
  Yuv420Image res(rgbx.width(), rgbx.height());
  // J420?
  libyuv::ARGBToI420(rgbx.data<0>(), static_cast<int>(rgbx.stride<0>()),
                     res.data<0>(), static_cast<int>(res.stride<0>()),
                     res.data<1>(), static_cast<int>(res.stride<1>()),
                     res.data<2>(), static_cast<int>(res.stride<2>()),
                     static_cast<int>(rgbx.width()),
                     static_cast<int>(rgbx.height()));

  return res;
}

Yuv420Image to_yuv420(const Yuv444Image &src) {
  Yuv420Image res(src.width(), src.height());

  libyuv::I444ToI420(src.data<0>(), static_cast<int>(src.stride<0>()),
                     src.data<1>(), static_cast<int>(src.stride<1>()),
                     src.data<2>(), static_cast<int>(src.stride<2>()),
                     res.data<0>(), static_cast<int>(res.stride<0>()),
                     res.data<1>(), static_cast<int>(res.stride<1>()),
                     res.data<2>(), static_cast<int>(res.stride<2>()),
                     static_cast<int>(src.width()),
                     static_cast<int>(src.height()));

  return res;
}

XRGBImage to_xrgb(const Yuv420Image &yuv) {
  XRGBImage res(yuv.width(), yuv.height());

  libyuv::I420ToARGB(yuv.data<0>(), static_cast<int>(yuv.stride<0>()),
                     yuv.data<1>(), static_cast<int>(yuv.stride<1>()),
                     yuv.data<2>(), static_cast<int>(yuv.stride<2>()),
                     res.data<0>(), static_cast<int>(res.stride<0>()),
                     static_cast<int>(yuv.width()),
                     static_cast<int>(yuv.height()));

  return res;
}

XRGBImage to_xrgb(const Yuv444Image &yuv) {
  XRGBImage res(yuv.width(), yuv.height());

  libyuv::I444ToARGB(yuv.data<0>(), static_cast<int>(yuv.stride<0>()),
                     yuv.data<1>(), static_cast<int>(yuv.stride<1>()),
                     yuv.data<2>(), static_cast<int>(yuv.stride<2>()),
                     res.data<0>(), static_cast<int>(res.stride<0>()),
                     static_cast<int>(yuv.width()),
                     static_cast<int>(yuv.height()));

  return res;
}

XRGBImage to_xrgb(const GrayscaleImage &gs) {
  XRGBImage res(gs.width(), gs.height());

  for (size_t i = 0; i < gs.height(); ++i) {
    ARGBPixel *p = res.row<0>(i);
    const uint8_t *y = gs.row<0>(i);
    for (size_t j = 0; j < gs.width(); ++j) {
      *p = ARGBPixel::mk(255, *y, *y, *y);
      ++y;
      ++p;
    }
  }
  return res;
}

ConvRes<Yuv420Image> to_yuv420(const Image &img) {
  switch (img.type()) {
  case ImageType::XRGB:
    return ConvRes<Yuv420Image>(inplace,
                                to_yuv420(*img.val<ImageType::XRGB>()));
  case ImageType::YUV420:
    return ConvRes<Yuv420Image>(img.val<ImageType::YUV420>());
  case ImageType::GRAYSCALE:
    return ConvRes<Yuv420Image>(inplace,
                                to_yuv420(*img.val<ImageType::GRAYSCALE>()));
  case ImageType::YUV444:
    return ConvRes<Yuv420Image>(inplace,
                                to_yuv420(*img.val<ImageType::YUV444>()));
  }
}

ConvRes<XRGBImage> to_xrgb(const Image &img) {
  switch (img.type()) {
  case ImageType::XRGB:
    return ConvRes<XRGBImage>(img.val<ImageType::XRGB>());
  case ImageType::YUV420:
    return ConvRes<XRGBImage>(inplace, to_xrgb(*img.val<ImageType::YUV420>()));
  case ImageType::GRAYSCALE:
    return ConvRes<XRGBImage>(inplace,
                              to_xrgb(*img.val<ImageType::GRAYSCALE>()));
  case ImageType::YUV444:
    return ConvRes<XRGBImage>(inplace, to_xrgb(*img.val<ImageType::YUV444>()));
  }
}

const inplace_t inplace{};
