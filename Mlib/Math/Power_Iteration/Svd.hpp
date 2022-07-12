#pragma once
#include <Mlib/Math/Power_Iteration/Power_Iteration.hpp>

namespace Mlib {

template <class TData>
void svd_u(
    const Array<TData>& a,
    Array<TData>& uT,
    Array<typename FloatType<TData>::value_type>& s,
    Array<TData>& vT,
    size_t seed = 1)
{
    assert(a.ndim() == 2);
    uT.resize(a.shape(0), a.shape(0));
    s.resize(uT.shape(0));
    // A=USV'
    // AA'=US²U'
    // V'=(1/S)U'A
    auto m = outer(a, a);
    for (size_t i = 0; i < uT.shape(0); ++i) {
        power_iteration(m, uT, s(i), i, seed);
    }
    vT = dot(uT, a);
    for (size_t r = 0; r < vT.shape(0); ++r) {
        s(r) = std::sqrt(s(r));
        for (size_t c = 0; c < vT.shape(1); ++c) {
            vT(r, c) /= s(r);
        }
    }
}

template <class TData>
void svd(
    const Array<TData>& a,
    Array<TData>& uT,
    Array<typename FloatType<TData>::value_type>& s,
    Array<TData>& vT,
    size_t seed = 1)
{
    assert(a.ndim() == 2);
    if (a.shape(0) > a.shape(1)) {
        svd_u(a.H(), vT, s, uT, seed);
    } else {
        svd_u(a, uT, s, vT, seed);
    }
}

}
