// -*- C++ -*-
#pragma once
#include "aligned_storage.h"
#include "indice_tuple.h"

enum class ColorSpace : unsigned char;

//==============================================================================

template <size_t _IDX, typename... _DIMS>
struct _get_nth_dims;

template <typename _HD, typename... _RST>
struct _get_nth_dims<0, _HD, _RST...> : public _HD {};

template <size_t _IDX, typename _HD, typename... _RST>
struct _get_nth_dims<_IDX, _HD, _RST...>
        : public _get_nth_dims<_IDX - 1, _RST...> {};

//==============================================================================

template <unsigned int _VSAMP, unsigned int _HSAMP = _VSAMP>
struct P {
    static constexpr unsigned int v = _VSAMP, h = _HSAMP;
};

//==============================================================================
// TODO

class PlanarImageBase {
   protected:
    unsigned int width_, height_;

    Aligned<uint8_t> data_;

   public:
    PlanarImageBase() : width_(0), height_(0), data_(nullptr) {}

    PlanarImageBase(PlanarImageBase &&rhs)
            : width_(rhs.width_),
              height_(rhs.height_),
              data_(std::move(rhs.data_)) {}

    PlanarImageBase(const PlanarImageBase &) = delete;

    PlanarImageBase &operator=(PlanarImageBase &&rhs) {
        data_ = std::move(rhs.data_);
        width_ = rhs.width_;
        height_ = rhs.height_;
        return *this;
    }

    PlanarImageBase &operator=(const PlanarImageBase &) = delete;

    bool is_empty() const noexcept { return data_.is_empty(); }

    unsigned int width() const noexcept { return width_; }

    unsigned int height() const noexcept { return height_; }
};

//------------------------------------------------------------------------------

template <ColorSpace IMAGE_TYPE_, typename PIXEL_TYPE_ = uint8_t,
          typename... DIMS_>
class PlanarImage : public PlanarImageBase {
    static_assert(sizeof...(DIMS_) > 0, "Cannot declare empty image");

    template <size_t... _Indices>
    size_t _offset(tuple_helper::tuple_indices<_Indices...>) const noexcept {
        return tuple_helper::sum((height<_Indices>() * stride<_Indices>())...);
    }

    size_t size() const noexcept { return offset<sizeof...(DIMS_)>(); }

   public:
    using PlanarImageBase::width;
    using PlanarImageBase::height;
    template <size_t N_>
    size_t offset() const noexcept {
        typedef typename tuple_helper::make_tuple_indices<N_>::type _Index;
        return _offset(_Index());
    }

    PlanarImage(unsigned int w, unsigned int h) : PlanarImageBase() {
        width_ = w;
        height_ = h;
        data_.alloc(size());
    }

    PlanarImage() : PlanarImageBase() {}

    PlanarImage(PlanarImage &&rhs) : PlanarImageBase(std::move(rhs)) {}
    PlanarImage(const PlanarImage &) = delete;

    PlanarImage &operator=(PlanarImage &&) = default;
    PlanarImage &operator=(const PlanarImage &) = delete;

    typedef PIXEL_TYPE_ pixel_type;

    static constexpr size_t num_planes = sizeof...(DIMS_);

    static constexpr ColorSpace type = IMAGE_TYPE_;

    static constexpr unsigned int max_h_samp = tuple_helper::max(DIMS_::h...);
    static constexpr unsigned int max_v_samp = tuple_helper::max(DIMS_::v...);

    template <size_t N_>
    using dim = _get_nth_dims<N_, DIMS_...>;

    template <size_t N_>
    unsigned int width() const noexcept {
        return up_div(width_, dim<N_>::v);
    }

    template <size_t N_>
    unsigned int height() const noexcept {
        return up_div(height_, dim<N_>::h);
    }

    template <size_t N_>
    size_t stride() const noexcept {
        return align(width<N_>() * sizeof(pixel_type));
    }

    template <size_t N_>
    size_t size() const noexcept {
        return (height<N_>() * stride<N_>());
    }

    template <size_t N_>
    uint8_t *data() noexcept {
        return data_.ptr() + offset<N_>();
    }

    template <size_t N_>
    const uint8_t *data() const noexcept {
        return data_.ptr() + offset<N_>();
    }

    template <size_t N_>
    pixel_type *row(size_t i) noexcept {
        return reinterpret_cast<pixel_type *>(data_.ptr() + offset<N_>() +
                                              i * stride<N_>());
    }

    template <size_t N_>
    const pixel_type *row(size_t i) const noexcept {
        return reinterpret_cast<const pixel_type *>(data_.ptr() + offset<N_>() +
                                                    i * stride<N_>());
    }
};

// TODO: packed image ()
