#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData, class Tf>
Array<TData> numerical_differentiation(const Tf& f, const Array<TData>& x, const TData& h=1e-3) {
    auto fx = f(x);

    Array<TData> JT(ArrayShape{x.length(), fx.length()});
    for (size_t i = 0; i < x.length(); ++i) {
        Array<TData> xi;
        xi = x;
        xi(i) += h;
        JT[i] = (f(xi) - fx) / h;
    }
    return JT.T();
}

template <size_t c, class TData, class Tf>
Array<TData> mixed_numerical_differentiation(const Tf& f, const FixedArray<TData, c>& x, const TData& h = 1e-3) {
    auto fx = f(x);

    Array<TData> JT(ArrayShape{ x.length(), fx.length() });
    for (size_t i = 0; i < x.length(); ++i) {
        FixedArray<TData, c> xi = x;
        xi(i) += h;
        JT[i] = (f(xi) - fx) / h;
    }
    return JT.T();
}

template <size_t r, size_t c, class TData, class Tf>
FixedArray<TData, r, c> numerical_differentiation(const Tf& f, const FixedArray<TData, c>& x, const TData& h = 1e-3) {
    FixedArray<TData, r> fx = f(x);

    FixedArray<TData, c, r> JT = uninitialized;
    for (size_t i = 0; i < x.length(); ++i) {
        FixedArray<TData, c> xi = x;
        xi(i) += h;
        JT[i] = (f(xi) - fx) / h;
    }
    return JT.T();
}

}
