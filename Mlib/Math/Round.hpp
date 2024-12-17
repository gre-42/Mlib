#pragma once
#include <cmath>
#include <concepts>
#include <type_traits>

namespace Mlib {

template <std::floating_point T>
constexpr T round(T val) {
    if (std::is_constant_evaluated()) {
        // From: https://github.com/MikeLankamp/fpm/issues/26
        return static_cast<T>((val >= 0.f) ? (val + 0.5f) : (val - 0.5f));
    } else {
        return std::round(val);
    }
}

}
