#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <std::floating_point T>
auto assert_finite(const T& v, const char* prefix) {
    if (!std::isfinite(v)) {
        throw std::runtime_error((std::stringstream() << prefix << " not finite: " << v).str());
    }
    return v;
}

template <std::floating_point T, size_t... tshape>
auto assert_finite(const FixedArray<T, tshape...>& v, const char* prefix) {
    if (!all(isfinite(v))) {
        throw std::runtime_error((std::stringstream() << prefix << " not finite: "  << v).str());
    }
    return v;
}

template <Scalar T>
auto assert_range(const T& v, const T& min, const T& max, const char* prefix) {
    if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(v)) {
            throw std::runtime_error((std::stringstream() << prefix << " is NAN").str());
        }
    }
    if ((v < min) || (v > max)) {
        throw std::runtime_error((std::stringstream() << prefix << " out of bounds: "  << v).str());
    }
    return v;
}

}
