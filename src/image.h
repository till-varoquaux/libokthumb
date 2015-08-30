// -*- C++ -*-
#pragma once
#include <cassert>
#include "src/aligned_storage.h"
#include "src/planar_image.h"
#include "src/okthumb.h"

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
#error "Unhandled endian-ness"
#endif
};

enum class ColorSpace : unsigned char {
    XRGB,
    YUV420,
    YUV444,
    GRAYSCALE  ///< AKA i400
};

constexpr inline bool is_plannar(ColorSpace ty) {
    return ty != ColorSpace::XRGB;
}

const char *to_string(ColorSpace);

template <ColorSpace>
struct ImageFormat;

template <>
struct ImageFormat<ColorSpace::YUV420> {
    typedef PlanarImage<ColorSpace::YUV420, uint8_t, P<1>, P<2>, P<2>> type;
};

template <>
struct ImageFormat<ColorSpace::YUV444> {
    typedef PlanarImage<ColorSpace::YUV444, uint8_t, P<1>, P<1>, P<1>> type;
};

template <>
struct ImageFormat<ColorSpace::GRAYSCALE> {
    typedef PlanarImage<ColorSpace::GRAYSCALE, uint8_t, P<1>> type;
};

template <>
struct ImageFormat<ColorSpace::XRGB> {
    typedef PlanarImage<ColorSpace::XRGB, ARGBPixel, P<1>> type;
};

template <ColorSpace ty>
using BaseImage = typename ImageFormat<ty>::type;

typedef BaseImage<ColorSpace::XRGB> XRGBImage;
typedef BaseImage<ColorSpace::YUV420> Yuv420Image;
typedef BaseImage<ColorSpace::YUV444> Yuv444Image;
typedef BaseImage<ColorSpace::GRAYSCALE> GrayscaleImage;

class Image : public PlanarImageBase {
    ColorSpace type_;

   public:
    Image() : PlanarImageBase(), type_(ColorSpace::YUV420) {}
    ~Image() = default;

    template <ColorSpace IMAGETYPE_, typename PIXEL_, typename... DIMS_>
    explicit Image(PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...> &&rhs)
            : PlanarImageBase(std::move(rhs)), type_(IMAGETYPE_) {
        static_assert(std::is_same<PlanarImage<IMAGETYPE_, PIXEL_, DIMS_...>,
                                   BaseImage<IMAGETYPE_>>::value,
                      "Internal error");
    }

    Image(Image &&rhs) : PlanarImageBase(std::move(rhs)), type_(rhs.type_) {}

    template <ColorSpace IMAGETYPE_, typename PIXEL_, typename... DIMS_>
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

    ColorSpace type() const { return type_; }

    template <ColorSpace TY_>
    const BaseImage<TY_> *val() const noexcept {
        if (TY_ == type_) {
            return reinterpret_cast<const BaseImage<TY_> *>(
                    static_cast<const PlanarImageBase *>(this));
        }
        return nullptr;
    }

    template <ColorSpace TY_>
    BaseImage<TY_> *val() noexcept {
        if (TY_ == type_) {
            return reinterpret_cast<BaseImage<TY_> *>(
                    static_cast<PlanarImageBase *>(this));
        }
        return nullptr;
    }
};
