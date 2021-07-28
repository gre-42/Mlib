#pragma once
#include <Mlib/Math/Power_Iteration/Svd.hpp>

namespace Mlib {

template <class TData>
Array<TData> pinv(const Array<TData>& a) {
    // a = u s v'
    // p = v 1/s u' = v/s u' = (1/s v')' u'
    Array<TData> uT;
    Array<typename FloatType<TData>::value_type> s;
    Array<TData> vT;
    svd(a, uT, s, vT);
    for (size_t r = 0; r < vT.shape(0); r++) {
        for (size_t c = 0; c < vT.shape(1); c++) {
            vT(r, c) /= s(r);
        }
    }
    return dot(vT.H(), uT);
}

template <class TData>
Array<TData> lstsq(const Array<TData>& a, const Array<TData>& b) {
    return dot(pinv(a), b);
}

}
