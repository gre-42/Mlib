#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib { namespace Sfm {

template <class TData>
Array<TData> homography_from_points(
    const Array<TData>& x, const Array<TData>& p)
{
    // p = x-prime = x'
    assert(all(x.shape() == p.shape()));
    assert(x.shape(1) == 3);
    Array<TData> M = zeros<TData>(ArrayShape{2 * x.shape(0) + 1, 9});
    for (size_t r = 0; r < x.shape(0); ++r) {
        M(2 * r + 0, 0) = -x(r, 0);
        M(2 * r + 0, 1) = -x(r, 1);
        M(2 * r + 0, 2) = -x(r, 2);
        M(2 * r + 1, 3) = -x(r, 0);
        M(2 * r + 1, 4) = -x(r, 1);
        M(2 * r + 1, 5) = -x(r, 2);

        M(2 * r + 0, 6) = p(r, 0) * x(r, 0);
        M(2 * r + 0, 7) = p(r, 0) * x(r, 1);
        M(2 * r + 0, 8) = p(r, 0);
        M(2 * r + 1, 6) = p(r, 1) * x(r, 0);
        M(2 * r + 1, 7) = p(r, 1) * x(r, 1);
        M(2 * r + 1, 8) = p(r, 1);
    }
    M(M.shape(0) - 1, M.shape(1) - 1) = 1;
    return lstsq_chol_1d(M, dirac_array<TData>(
        ArrayShape{M.shape(0)},
        ArrayShape{M.shape(0) - 1})).reshaped(ArrayShape{3, 3});
}

}}
