#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
void inverse_iteration_symm(
    const Array<TData>& a,
    Array<TData>& u,
    typename FloatType<TData>::value_type& s,
    const TData& alpha = 0,
    const TData& beta = 0,
    unsigned int seed = 1)
{
    assert(a.ndim() == 2);
    assert(a.shape(0) == a.shape(1));
    u = random_array3<TData>(ArrayShape{a.shape(0)}, seed);

    for (size_t n = 0; n < 30 * a.shape(0); n++) {
        // u = lstsq_chol_1d(a, u, float(1e-1));
        u = solve_symm_1d(a, u, alpha, beta).value();
        s = 1 / std::sqrt(sum(norm(u)));
        u *= s;
    }
}

}
