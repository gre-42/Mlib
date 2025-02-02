#pragma once
#include <Mlib/Math/Power_Iteration/Inverse_Iteration.hpp>
#include <Mlib/Math/Power_Iteration/Power_Iteration.hpp>

namespace Mlib {

template <class TData>
typename FloatType<TData>::value_type cond(const Array<TData>& a) {
    Array<TData> m;
    if (a.shape(0) < a.shape(1)) {
        m = outer(a, a);
    } else {
        m = dot(a.H(), a);
    }
    Array<TData> u0;
    Array<TData> uT1{ArrayShape{1, m.shape(0)}};
    typename FloatType<TData>::value_type s0;
    typename FloatType<TData>::value_type s1;
    inverse_iteration_symm(m, u0, s0);
    power_iteration(m, uT1, s1, 0);
    return s1 / s0;  // no square-root
}

}
