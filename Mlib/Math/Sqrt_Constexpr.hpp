#pragma once
#include <limits>   

namespace Mlib {

namespace Detail {

template <class T>
T constexpr sqrt_newton_raphson(T x, T curr, T prev) {
    return curr == prev
        ? curr
        : sqrt_newton_raphson<T>(x, T(0.5) * (curr + x / curr), curr);
}

}

// Constexpr version of the square root
// Return value:
//   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
//   - Otherwise, returns NaN
// From: https://stackoverflow.com/a/34134071/2292832
// 
template <class T>
T constexpr sqrt_constexpr(T x)
{
    return x >= 0 && x < std::numeric_limits<T>::infinity()
        ? Detail::sqrt_newton_raphson<T>(x, x, 0)
        : std::numeric_limits<T>::quiet_NaN();
}

}
