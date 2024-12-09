#pragma once
#include <Mlib/Math/Fixed_Cholesky.hpp>

namespace Mlib::Sfm {

template <class TData>
FixedArray<TData, 3, 3> homography_from_points(
    const Array<FixedArray<TData, 2>>& x, const Array<FixedArray<TData, 2>>& p)
{
    // p = x-prime = x'
    assert(x.ndim() == 1);
    assert(all(x.shape() == p.shape()));
    Array<TData> M = zeros<TData>(ArrayShape{2 * x.shape(0) + 1, 9});
    for (size_t r = 0; r < x.shape(0); ++r) {
        M(2 * r + 0, 0) = -x(r)(0);
        M(2 * r + 0, 1) = -x(r)(1);
        M(2 * r + 0, 2) = -1; // -x(r)(2);
        M(2 * r + 1, 3) = -x(r)(0);
        M(2 * r + 1, 4) = -x(r)(1);
        M(2 * r + 1, 5) = -1; // -x(r, 2);

        M(2 * r + 0, 6) = p(r)(0) * x(r)(0);
        M(2 * r + 0, 7) = p(r)(0) * x(r)(1);
        M(2 * r + 0, 8) = p(r)(0);
        M(2 * r + 1, 6) = p(r)(1) * x(r)(0);
        M(2 * r + 1, 7) = p(r)(1) * x(r)(1);
        M(2 * r + 1, 8) = p(r)(1);
    }
    M(M.shape(0) - 1, M.shape(1) - 1) = 1;
    return FixedArray<double, 3, 3>{
        lstsq_chol_1d(M.template casted<double>(), dirac_array<double>(
            ArrayShape{ M.shape(0) },
            ArrayShape{ M.shape(0) - 1 })).value().reshaped(ArrayShape{ 3, 3 })}
        .template casted<TData>();
}

}
