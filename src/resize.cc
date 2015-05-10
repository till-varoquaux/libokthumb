#include "resize.h"
#include "resize_tmpl.h"
#include "include/libyuv.h"

// As of 1037:
//  The yuv box filter is faster if width is a multiple of 16
//  The Argb scaler does not support Box filtering

static libyuv::FilterModeEnum to_filter_mode(const scale_method mthd) {
  switch (mthd) {
  case scale_method::FASTEST:
      return libyuv::kFilterBilinear;
  case scale_method::NORMAL:
      return libyuv::kFilterBox;
  case scale_method::BEST:
      return libyuv::kFilterBox;
  }
}

static bool resize_plane(const uint8_t *src, int src_stride, int src_width,
                  int src_height, uint8_t *dst, int dst_stride, int dst_width,
                  int dst_height, const scale_method mthd) {
  ScalePlane(src, src_stride, src_width, src_height, dst, dst_stride, dst_width,
             dst_height, to_filter_mode(mthd));
  return true;
}

static bool resize_rgbx(const uint8_t *src, int src_stride, int src_width,
                 int src_height, uint8_t *dst, int dst_stride, int dst_width,
                 int dst_height, const scale_method mthd) {
  int err = ARGBScale(src, src_stride, src_width, src_height, dst, dst_stride,
                      dst_width, dst_height, to_filter_mode(mthd));
  return !err;
}

Image resize(const Image &img, unsigned int w, unsigned int h,
             scale_method mthd) {
  return resize<resize_rgbx, resize_plane>(img, w, h, mthd);
}
