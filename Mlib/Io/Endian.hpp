#pragma once
#include <array>

namespace Mlib {

template <class T>
T swap_endianness(const T& v) {
    const auto& a = reinterpret_cast<const std::array<char, sizeof(T)>&>(v);
    std::array<char, sizeof(T)> result;
    for (size_t i = 0; i < sizeof(T); ++i) {
        result[i] = a[sizeof(T) - i - 1];
    }
    return reinterpret_cast<const T&>(result);
}

}
