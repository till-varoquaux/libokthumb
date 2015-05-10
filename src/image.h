// -*- C++ -*-
#pragma once
#include <cassert>
#include "aligned_storage.h"
#include "planar_image.h"

/**
 *   <---------------------stride------------------------>
 * origin
 *   +---------------------------------------------------+
 *   |      .<--------------width------------->.         |
 *   |      .                                  .         |
 *   |      +----------------------------------+......   |
 *   |      |                                  |     |   |
 *   |      |                                  |     h   |
 *   |      |                                  |     e   |
 *   |      |                                  |     i   |
 *   |      |                                  |     g   |
 *   |      |                                  |     h   |
 *   |      |                                  |     t   |
 *   |      |                                  |     |   |
 *   |      +----------------------------------+.....|   |
 *   +---------------------------------------------------+
  *                                                origin+size
 */

// Move to argb

// Libyuv only has the whole set of conversions for argb

struct __attribute__((packed)) ARGBPixel {
// 32bit xrgb / argb
//  ARGB little endian (bgra in memory)
#if __BYTE_ORDER == __LITTLE_ENDIAN
  // x86 and friends
  unsigned char b;
  unsigned char g;
  unsigned char r;
  unsigned char a;
  static ARGBPixel mk(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return {b, g, r, a};
  }

// #if __BYTE_ORDER == __BIG_ENDIAN
//   unsigned char a;
//   unsigned char r;
//   unsigned char g;
//   unsigned char b;
//   static ARGBPixel mk(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
//     return {a, r, g, b};
//   }

#else
#error "Unknown endian-ness"
#endif
};

enum class ImageType : unsigned char { XRGB, YUV420, YUV444, GRAYSCALE };

constexpr bool is_plannar(ImageType ty) { return ty != ImageType::XRGB; }

const char *to_string(ImageType);

template <ImageType> struct ImageFormat;

template <> struct ImageFormat<ImageType::YUV420> {
  typedef PlanarImage<ImageType::YUV420, uint8_t, P<1>, P<2>, P<2>> type;
};

template <> struct ImageFormat<ImageType::YUV444> {
  typedef PlanarImage<ImageType::YUV444, uint8_t, P<1>, P<1>, P<1>> type;
};

template <> struct ImageFormat<ImageType::GRAYSCALE> {
  typedef PlanarImage<ImageType::GRAYSCALE, uint8_t, P<1>> type;
};

template <> struct ImageFormat<ImageType::XRGB> {
  typedef PlanarImage<ImageType::XRGB, ARGBPixel, P<1>> type;
};

template <ImageType ty> using BaseImage = typename ImageFormat<ty>::type;

typedef BaseImage<ImageType::XRGB> XRGBImage;
typedef BaseImage<ImageType::YUV420> Yuv420Image;
typedef BaseImage<ImageType::YUV444> Yuv444Image;
typedef BaseImage<ImageType::GRAYSCALE> GrayscaleImage;

class Image : public PlanarImageBase {
  ImageType type_;

public:
  Image() : PlanarImageBase(), type_(ImageType::YUV420) {}
  ~Image() = default;

  template <ImageType IMAGETYPE_, typename PIXEL_, typename... DIMS_>
  Image(PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...> &&rhs)
      : PlanarImageBase(std::move(rhs)), type_(IMAGETYPE_) {
    static_assert(std::is_same<PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...>,
                               BaseImage<IMAGETYPE_>>::value,
                  "Internal error");
  }

  Image(Image &&rhs) : PlanarImageBase(std::move(rhs)), type_(rhs.type_) {}

  template <ImageType IMAGETYPE_, typename PIXEL_, typename... DIMS_>
  Image &operator=(PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...> &&rhs) {
    static_assert(std::is_same<PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...>,
                               BaseImage<IMAGETYPE_>>::value,
                  "Internal error");
    PlanarImageBase::operator=(std::move(rhs));
    type_ = IMAGETYPE_;
    return *this;
  }

  Image &operator=(Image &&rhs) {
    PlanarImageBase::operator=(std::move(rhs));
    type_ = rhs.type_;
    return *this;
  }

  ImageType type() const { return type_; }

  template <ImageType TY_> const BaseImage<TY_> *val() const noexcept {
    if (TY_ == type_) {
      return reinterpret_cast<const BaseImage<TY_> *>(
          static_cast<const PlanarImageBase *>(this));
    }
    return nullptr;
  }

  template <ImageType TY_> BaseImage<TY_> *val() noexcept {
    if (TY_ == type_) {
      return reinterpret_cast<BaseImage<TY_> *>(
          static_cast<PlanarImageBase *>(this));
    }
    return nullptr;
  }
};

// TODO: better naming etc for this class
struct inplace_t {};
extern const inplace_t inplace;

template <typename T_> class ConvRes {
  bool ref_;
  union {
    const T_ *ptr_;
    T_ val_;
  };

public:
  template <typename... Args_>
  ConvRes(const inplace_t &, Args_ &&... args)
      : ref_(false) {
    new (&val_) T_(std::forward<Args_>(args)...);
  }

  ConvRes(const T_ *ptr) : ref_(true), ptr_(ptr) {}

  ConvRes(ConvRes &&l) : ref_(l.ref_) {
    if (ref_) {
      ptr_ = l.ptr_;
    } else {
      new (&val_) T_(std::forward<T_>(l.val_));
    }
  }

  const T_ &operator*() { return (ref_) ? *ptr_ : val_; }

  ~ConvRes() {
    if (!ref_) {
      val_.~T_();
    }
  }
};

GrayscaleImage to_grayscale(const Yuv420Image &);
GrayscaleImage to_grayscale(const Yuv444Image &);

XRGBImage to_xrgb(const Yuv420Image &);
XRGBImage to_xrgb(const Yuv444Image &);
XRGBImage to_xrgb(const GrayscaleImage &);

Yuv420Image to_yuv420(const Yuv444Image &);
Yuv420Image to_yuv420(const GrayscaleImage &);
Yuv420Image to_yuv420(const XRGBImage &);

ConvRes<Yuv420Image> to_yuv420(const Image &);
ConvRes<XRGBImage> to_xrgb(const Image &);
