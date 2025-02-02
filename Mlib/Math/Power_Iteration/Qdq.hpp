#pragma once
#include <Mlib/Math/Power_Iteration/Power_Iteration.hpp>

namespace Mlib {

/*
 * Diagonalization of a symmetric matrix.
 */
template <class TData>
void qdq(
    const Array<TData>& a,
    Array<TData>& q,
    Array<typename FloatType<TData>::value_type>& s,
    size_t seed = 1)
{
    assert(a.ndim() == 2);
    assert(a.shape(0) == a.shape(1));
    q.resize[a.shape(0)](a.shape(0));
    s.resize(q.shape(0));
    for (size_t i=0; i<a.shape(0); i++) {
        power_iteration(a, q, s(i), i, seed);
    }
}

}
