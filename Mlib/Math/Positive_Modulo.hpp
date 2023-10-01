#pragma once
#include <cmath>
#include <concepts>

namespace Mlib {

// From: https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
template <std::floating_point TData>
TData positive_modulo(TData i, TData n) {
    return std::fmod(std::fmod(i, n) + n, n);
}

// From: https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

}
