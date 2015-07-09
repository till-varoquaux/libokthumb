// -*- C++ -*-
#pragma once

#include "image.h"

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
  explicit ConvRes(const inplace_t &, Args_ &&... args)
      : ref_(false) {
    new (&val_) T_(std::forward<Args_>(args)...);
  }

  explicit ConvRes(const T_ *ptr) : ref_(true), ptr_(ptr) {}

  ConvRes(ConvRes &&l) : ref_(l.ref_) {
    if (ref_) {
      ptr_ = l.ptr_;
    } else {
      new (&val_) T_(std::forward<T_>(l.val_));
    }
  }

  const T_ &operator*() const { return (ref_) ? *ptr_ : val_; }

  ~ConvRes() {
    if (!ref_) {
      val_.~T_();
    }
  }
};

const ConvRes<Yuv420Image> to_yuv420(const Image &);
const ConvRes<XRGBImage> to_xrgb(const Image &);

