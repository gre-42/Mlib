#pragma once
#include <Mlib/Math/Funpack.hpp>
#include <concepts>

namespace Mlib {

template <class TData, class TAlpha>
auto lerp(const TData& a, const TData& b, const TAlpha& alpha) {
    return (TData)(funpack(a) + alpha * funpack(b - a));
}

template <class TData, class TAlpha>
auto lerp_symm(const TData& a, const TData& b, const TAlpha& alpha) {
    if (alpha < 0.5) {
        return lerp(a, b, alpha);
    } else {
        return lerp(b, a, 1 - alpha);
    }
}

template <class TData, std::floating_point TAlpha>
auto lerp11(const TData& a, const TData& b, const TAlpha& alpha) {
    return ((1 - alpha) / 2) * funpack(a) + ((alpha + 1) / 2) * funpack(b);
}

}
