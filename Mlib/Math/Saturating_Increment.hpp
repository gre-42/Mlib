#pragma once
#include <concepts>
#include <limits>

namespace Mlib {

template <std::integral T>
constexpr T saturating_increment(T val) noexcept {
    return (val != std::numeric_limits<T>::max())
        ? val + 1
        : val;
}

}
