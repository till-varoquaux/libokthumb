// -*- C++ -*-
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

template <resize_plane_fn_t _FN, typename _IMG, typename _TupleIndexes>
struct resize_helper_impl_t;

template <resize_plane_fn_t _FN, typename _IMG, size_t... _IDX>
struct resize_helper_impl_t<_FN, _IMG, tuple_indices<_IDX...>> {
    static bool resize(const _IMG &src, _IMG *dst, const scale_method mthd) {
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

template <resize_plane_fn_t _FN, typename _IMG>
using resize_helper_t = resize_helper_impl_t<
        _FN, _IMG, typename make_tuple_indices<_IMG::num_planes>::type>;

template <resize_plane_fn_t _FN, typename _IMG>
_IMG resize_planar(const _IMG &src, unsigned int w, unsigned int h,
                   const scale_method mthd) {
    _IMG res(w, h);
    bool ok = resize_helper_t<_FN, _IMG>::resize(src, &res, mthd);
    if (ok) {
        return res;
    } else {
        return _IMG();
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
            return Image(resize_planar<_RESIZE_GRAY>(
                    *img.val<ColorSpace::YUV420>(), w, h, mthd));
        case ColorSpace::YUV444:
            return Image(resize_planar<_RESIZE_GRAY>(
                    *img.val<ColorSpace::YUV444>(), w, h, mthd));
        case ColorSpace::GRAYSCALE:
            return Image(resize_planar<_RESIZE_GRAY>(
                    *img.val<ColorSpace::GRAYSCALE>(), w, h, mthd));
    }
}
