// -*- C++ -*-
#pragma once
#include <type_traits>
#include <cassert>
#include <iostream>

template <unsigned int _Max_v, class _Underlying = unsigned char>
class bounded_int {
    static_assert(sizeof(_Underlying) < sizeof(unsigned int),
                  "Underlying type too big");
    static_assert(_Max_v <= ((1L << (8 * sizeof(_Underlying))) - 1),
                  "Max value too big for underlying type.");

   private:
    _Underlying value;

   public:
    explicit bounded_int(_Underlying v = 0) : value(v) { assert(v <= _Max_v); }
    operator _Underlying() const { return value; }

    _Underlying val() const { return value; }

    bounded_int<_Max_v, _Underlying> &operator=(_Underlying v) {
        assert(v <= _Max_v);
        value = v;
        return *this;
    }
};

namespace std {
template <unsigned int _Max_v, class _Underlying>
ostream &operator<<(ostream &strm, const bounded_int<_Max_v, _Underlying> &b) {
    return strm << static_cast<const unsigned int>(b);
}

template <unsigned int _Max_v, class _Underlying>
istream &operator>>(istream &strm, bounded_int<_Max_v, _Underlying> &b) {
    unsigned int tmp;
    strm >> tmp;
    if (tmp > _Max_v) {
        strm.setstate(std::ios::failbit);
    } else {
        b = static_cast<const _Underlying>(tmp);
    }
    return strm;
}
}
