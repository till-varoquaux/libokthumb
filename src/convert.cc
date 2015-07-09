#include "convert.h"
#include <include/libyuv.h>

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


// All of libyuv conversion functions are called the same way:
// f(src.data<0>, src.stride<0>,..., src.data<n>, src.stride<n>,
//   dst.data<0>, dst.stride<0>,..., dst.data<n>, dst.stride<n>,
//   width, height)
// This templatizes a lot of that boilerplate code...

template <size_t N>
struct GetDataOp {
    template <typename _T>
    static uint8_t *get(_T &v) { return v.template data<N>(); }

    template <typename _T>
    static const uint8_t *get(const _T &v) { return v.template data<N>(); }
};

template <size_t N>
struct GetStrideOp {
    template <typename _T>
    static int get(const _T &v) {
      return static_cast<int>(v.template stride<N>());
    }
};

//------------------------------------------------------------------------------

template <typename... _T> struct op_list {};

template <size_t _Ep, class _OpList> struct __make_oplist_imp;

template <size_t _Ep, typename... _Ops>
struct __make_oplist_imp<_Ep, op_list<_Ops...>> {
  typedef typename __make_oplist_imp<
      _Ep - 1, op_list<GetDataOp<_Ep - 1>, GetStrideOp<_Ep - 1>, _Ops...>>::type
      type;
};

template <typename ... _Ops>
struct __make_oplist_imp<0, op_list<_Ops...>> {
  typedef op_list<_Ops...> type;
};

//------------------------------------------------------------------------------

template <typename T>
using make_oplist = typename __make_oplist_imp<T::num_planes, op_list<>>::type;

template <typename _SRC, typename _DST, typename _F, typename... _SRC_OPS,
          typename... _DST_OPS>
_DST convert_impl(const _F *f, const _SRC &src, const op_list<_SRC_OPS...>,
                  const op_list<_DST_OPS...>) {
    _DST res(src.width(), src.height());
    f(_SRC_OPS::get(src)..., _DST_OPS::get(res)...,
      static_cast<int>(src.width()),
      static_cast<int>(src.height()));
    return res;
}

template <typename _DST, typename _F, typename _SRC>
_DST convert(const _F *f, const _SRC &src) {
  return convert_impl<_SRC, _DST>(f, src, make_oplist<_SRC>(),
                                  make_oplist<_DST>());
}


const ConvRes<Yuv420Image> to_yuv420(const Image &img) {
  switch (img.type()) {
  case ImageType::XRGB:
    return ConvRes<Yuv420Image>(inplace,
                                convert<Yuv420Image>(libyuv::ARGBToI420,
                                                     *img.val<ImageType::XRGB>()));
  case ImageType::YUV420:
    return ConvRes<Yuv420Image>(img.val<ImageType::YUV420>());
  case ImageType::GRAYSCALE:
    return ConvRes<Yuv420Image>(
        inplace, convert<Yuv420Image>(libyuv::I400ToI420,
                                      *img.val<ImageType::GRAYSCALE>()));
  case ImageType::YUV444:
      return ConvRes<Yuv420Image>(
        inplace, convert<Yuv420Image>(libyuv::I444ToI420,
                                      *img.val<ImageType::YUV444>()));
  }
}

const ConvRes<XRGBImage> to_xrgb(const Image &img) {
  switch (img.type()) {
  case ImageType::XRGB:
    return ConvRes<XRGBImage>(img.val<ImageType::XRGB>());
  case ImageType::YUV420:
    return ConvRes<XRGBImage>(
        inplace,
        convert<XRGBImage>(libyuv::I420ToARGB, *img.val<ImageType::YUV420>()));
  case ImageType::GRAYSCALE:
    return ConvRes<XRGBImage>(
        inplace, convert<XRGBImage>(libyuv::I400ToARGB,
                                    *img.val<ImageType::GRAYSCALE>()));
  case ImageType::YUV444:
    return ConvRes<XRGBImage>(
        inplace, convert<XRGBImage>(libyuv::I444ToARGB,
                                    *img.val<ImageType::YUV444>()));
  }
}


static GrayscaleImage
    to_grayscale(const Yuv420Image &yuv) {
  GrayscaleImage res(yuv.width(), yuv.height());
  assert(yuv.size<0>() == res.size<0>());
  memcpy(res.data<0>(), yuv.data<0>(), res.size<0>());
  return res;
}

static GrayscaleImage to_grayscale(const Yuv444Image &yuv) {
  GrayscaleImage res(yuv.width(), yuv.height());
  assert(yuv.size<0>() == res.size<0>());
  memcpy(res.data<0>(), yuv.data<0>(), res.size<0>());
  return res;
}

static GrayscaleImage to_grayscale(const XRGBImage &xrgb) {
  GrayscaleImage res(xrgb.width(), xrgb.height());
  libyuv::ARGBToI400(xrgb.data<0>(), static_cast<int>(xrgb.stride<0>()),
                     res.data<0>(), static_cast<int>(res.stride<0>()),
                     static_cast<int>(xrgb.width()),
                     static_cast<int>(xrgb.height()));
  return res;
}

const inplace_t inplace{};
