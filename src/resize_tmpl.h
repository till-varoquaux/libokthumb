// -*- C++ -*-
// This file abstracts away the base logic for resizing. Given one planar
// resizing function and one for XRGB this generate the whole family of resize
// functions for all our image types.
#pragma once
#include "src/image.h"
#include "src/indice_tuple.h"

using tuple_helper::make_tuple_indices;
using tuple_helper::tuple_indices;

typedef bool(resize_plane_fn_t)(const uint8_t *src, int src_stride,
                                int src_width, int src_height, uint8_t *dst,
                                int dst_stride, int dst_width, int dst_height,
                                const scale_method mthd);

typedef bool(resize_fn_t)(const Image &img, Image *res, unsigned int w,
                          unsigned int h, scale_method mthd);

template <resize_plane_fn_t _FN, typename _SRC, typename _DST,
          typename _TupleIndexes>
struct resize_helper_impl_t;

template <resize_plane_fn_t _FN, typename _SRC, typename _DST, size_t... _IDX>
struct resize_helper_impl_t<_FN, _SRC, _DST, tuple_indices<_IDX...>> {
    static bool resize(const _SRC &src, _DST *dst, const scale_method mthd) {
        return tuple_helper::all(
                _FN(src.template data<_IDX>(),
                    static_cast<int>(src.template stride<_IDX>()),
                    static_cast<int>(src.template width<_IDX>()),
                    static_cast<int>(src.template height<_IDX>()),
                    dst->template data<_IDX>(),
                    static_cast<int>(dst->template stride<_IDX>()),
                    static_cast<int>(dst->template width<_IDX>()),
                    static_cast<int>(dst->template height<_IDX>()), mthd)...);
    }
};

template <resize_plane_fn_t _FN, typename _SRC, typename _DST>
using resize_helper_t = resize_helper_impl_t<
    _FN, _SRC, _DST, typename make_tuple_indices<_SRC::num_planes>::type>;

template <resize_plane_fn_t _FN, ColorSpace _DST_COLORSPACE, typename _SRC>
BaseImage<_DST_COLORSPACE> resize_planar(const _SRC &src, unsigned int w,
                                         unsigned int h,
                                         const scale_method mthd) {
    typedef BaseImage<_DST_COLORSPACE> _DST;
    _DST res(w, h);
    bool ok = resize_helper_t<_FN, _SRC, _DST>::resize(src, &res, mthd);
    if (ok) {
        return res;
    } else {
        return _DST();
    }
}

template <resize_plane_fn_t _FN>
XRGBImage resize_rgbx(const XRGBImage &src, unsigned int w, unsigned int h,
                      const scale_method mthd) {
    XRGBImage res(w, h);
    bool ok = _FN(src.data<0>(), static_cast<int>(src.stride<0>()),
                  static_cast<int>(src.width()), static_cast<int>(src.height()),
                  res.data<0>(), static_cast<int>(res.stride<0>()),
                  static_cast<int>(w), static_cast<int>(h), mthd);
    if (ok) {
        return res;
    } else {
        return XRGBImage();
    }
}

template <resize_plane_fn_t _RESIZE_XRGB, resize_plane_fn_t _RESIZE_GRAY>
Image resize(const Image &img, unsigned int w, unsigned int h,
             scale_method mthd) {
    if (img.is_empty()) {
        return Image();
    }
    switch (img.type()) {
        case ColorSpace::XRGB:
            return Image(resize_rgbx<_RESIZE_XRGB>(*img.val<ColorSpace::XRGB>(),
                                                   w, h, mthd));
        case ColorSpace::YUV420:
            return Image(resize_planar<_RESIZE_GRAY, ColorSpace::YUV420>(
                    *img.val<ColorSpace::YUV420>(), w, h, mthd));
        case ColorSpace::YUV444:
            // Do the colorspace conversion here...
            return Image(resize_planar<_RESIZE_GRAY, ColorSpace::YUV420>(
                    *img.val<ColorSpace::YUV444>(), w, h, mthd));
        case ColorSpace::GRAYSCALE:
            return Image(resize_planar<_RESIZE_GRAY, ColorSpace::GRAYSCALE>(
                    *img.val<ColorSpace::GRAYSCALE>(), w, h, mthd));
    }
}
