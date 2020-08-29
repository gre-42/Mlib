#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData, class Tf>
Array<TData> numerical_differentiation(const Tf& f, const Array<TData>& x, const TData& h=1e-3) {
    auto fx = f(x);

    Array<TData> JT(ArrayShape{x.length(), fx.length()});
    for(size_t i = 0; i < x.length(); ++i) {
        Array<TData> xi;
        xi = x;
        xi(i) += h;
        JT[i] = (f(xi) - fx) / h;
    }
    return JT.T();
}

}
