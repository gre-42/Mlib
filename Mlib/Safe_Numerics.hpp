#pragma once
#include <Mlib/Array/Base_Dense_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <complex>
#include <stdexcept>
#include <string>

namespace Mlib {

template <class TData>
class Array;

template <class T>
struct SafeLimits {
    static const T zero_threshold;
    static const T lower_threshold;
    static const T upper_threshold;
};

template <class TScalar>
void assert_normal_scalar(const TScalar& x) {
    if (std::isnan(x)) {
        return;
    }
    if (!std::isfinite(x)) {
        return;
    }
    if (std::abs(x) < SafeLimits<TScalar>::zero_threshold) {
        return;
    }
    if (std::abs(x) < SafeLimits<TScalar>::lower_threshold) {
        THROW_OR_ABORT("Value is small: " + std::to_string(x));
    }
    if (std::abs(x) > SafeLimits<TScalar>::upper_threshold) {
        THROW_OR_ABORT("Value is large: " + std::to_string(x));
    }
}

inline void assert_normal(double x) { assert_normal_scalar(x); }
inline void assert_normal(float x) { assert_normal_scalar(x); }

template <class T>
void assert_normal(const std::complex<T>& x) {
    assert_normal_scalar(x.real());
    assert_normal_scalar(x.imag());
}

template <class TDerived, class TData>
void assert_normal(const BaseDenseArray<TDerived, TData>& x) {
    for (const auto& v : x->flat_iterable()) {
        assert_normal(v);
    }
}

}
