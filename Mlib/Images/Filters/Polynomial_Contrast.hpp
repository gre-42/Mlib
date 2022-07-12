#pragma once
#include <Mlib/Images/Mesh_Coordinates/Vandermonde.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> polynomial_contrast(const Array<Array<TData>>& x, const Array<TData>& weights, const Array<TData>& contrast, size_t poly_degree) {
    assert(x.ndim() == 1);
    assert(all(x(0).shape() == weights.shape()));
    Array<Array<TData>> V = vanderNd(x, poly_degree);
    assert(V.length() == contrast.length());
    Array<TData> M{ ArrayShape{ V.length(), V.length() }};
    for (size_t r = 0; r < M.shape(0); ++r) {
        for (size_t c = 0; c < M.shape(0); ++c) {
            if (r <= c) {
                M(r, c) = sum(V(r) * V(c) * weights);
            } else {
                M(r, c) = M(c, r);
            }
        }
    }
    Array<TData> m = solve_symm_1d(M, contrast).value();
    Array<TData> result = full<TData>(x(0).shape(), m(0));
    for (size_t i = 1; i < V.shape(0); ++i) {
        result += m(i) * V(i);
    }
    result *= weights;
    return result;
}

}
