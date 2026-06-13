#pragma once
#include <concepts>

namespace Mlib {

/**
 * From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
 */
template <std::integral T>
constexpr T int_log2(T n) {
    T result = 0;
    while (n >>= 1) ++result;
    return result;
}

}
