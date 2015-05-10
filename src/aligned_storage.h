// -*- C++ -*-
// Pointer to an aligned segment of memory. We use a bit of padding to align the
// memory
//          ------ ptr_
//         /
// 1000000xxxxxx
//  \------------ Beginning of allocated memory

#pragma once
#include <type_traits>
#include <memory>
#include <cassert>
#include <cstring>

// This would be super-seeded by:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3396.htm

static const size_t ALIGNMENT = 32;

template <typename T,
          typename = typename std::enable_if<std::is_unsigned<T>::value>::type>
constexpr T align(T t, T alignment = static_cast<T>(ALIGNMENT)) {
  return (t + (alignment - 1)) & -alignment;
}

// Rounds up the result of a division...
template <typename T,
          typename = typename std::enable_if<std::is_unsigned<T>::value>::type>
constexpr T up_div(T num, T denom) {
  return (num + (denom - 1)) / denom;
}

template <typename _T> class Aligned {
  static_assert(ALIGNMENT % alignof(_T) == 0,
                "Alignement should be a multiple of the alignment of 'type'");
  static_assert(std::is_trivial<_T>::value, "Supplied type should be trivial");

  unsigned char *ptr_;

public:
  Aligned() : ptr_(nullptr) {}

  explicit Aligned(std::nullptr_t) : ptr_(nullptr) {}

  explicit Aligned(Aligned &&other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }

  Aligned &operator=(Aligned &&other) {
    std::swap(other.ptr_, ptr_);
    return *this;
  }

  Aligned(size_t sz) : Aligned() { alloc(sz); }

  void clear() {
    if (ptr_) {
      unsigned char *raw = static_cast<unsigned char *>(ptr_) - 1;
      while (!*raw) {
        raw--;
      }
      delete[](raw);
      ptr_ = nullptr;
    }
  }

  ~Aligned() { clear(); }

  bool is_empty() const { return !ptr_; }

  void alloc(size_t sz) {
    assert(!ptr_);
    if (sz > 0) {
      unsigned char *raw = new unsigned char[sizeof(_T) * sz + ALIGNMENT + 1];
#ifndef NDEBUG
      memset(raw, 0, sizeof(_T) * sz + ALIGNMENT + 1);
#endif
      *raw = 1;
      ++raw;

      while (reinterpret_cast<size_t>(raw) % ALIGNMENT != 0) {
        *raw = 0;
        ++raw;
      }
      ptr_ = raw;
    }
  }

  _T *ptr() { return reinterpret_cast<_T *>(ptr_); }

  const _T *ptr() const { return reinterpret_cast<const _T *>(ptr_); }
};
