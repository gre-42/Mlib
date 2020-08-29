#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

/*
 * http://rosettacode.org/wiki/Cholesky_decomposition#Python
 */
template <class TData, size_t n>
FixedArray<TData, n, n> cholesky(const FixedArray<TData, n, n>& A) {
    FixedArray<TData, n, n> L;
    for(size_t i = 0; i < n; ++i) {
        for(size_t j = 0; j <= i; ++j) {
            TData s = 0;
            for(size_t k = 0; k < j; ++k) {
                s += L(i, k) * conju(L(j, k));
            }
            L(i, j) = (i == j) ? std::sqrt(A(i, i) - s) : ((A(i, j) - s) / L(j, j));
        }
    }
    return L;
}

/*
 * http://en.wikipedia.org/wiki/Triangular_matrix
 * backward substitution
 */
template <class TData, size_t m, size_t n>
FixedArray<TData, m, n> solve_LU(
    const FixedArray<TData, m, m>& L,
    const FixedArray<TData, m, m>& U,
    const FixedArray<TData, m, n>& B)
{
    // Ax = b -> LUx = b. Then y is defined to be Ux
    FixedArray<TData, m, n> x;
    FixedArray<TData, m, n> y;
    for(size_t v = 0; v < n; ++v) {
        // Forward solve Ly = b
        for(size_t i = 0; i < m; ++i) {
            y(i, v) = B(i, v);
            for(size_t j = 0; j < i; ++j) {
                y(i, v) -= L(i, j) * y(j, v);
            }
            y(i, v) /= L(i, i);
        }
        // Backward solve Ux = y
        for(size_t i = m - 1; i != SIZE_MAX; --i) {
            x(i, v) = y(i, v);
            for(size_t j = i + 1; j < m; ++j) {
                x(i, v) -= U(i, j) * x(j, v);
            }
            x(i, v) /= U(i, i);
        }
    }
    return x;
}

/*
 * Solves the equation A⁻¹B for a symmetric matrix A using Cholesky
 * decomposition.
 * [C    ] * x = [D     ]
 * [eps*I]       [eps*x0]
 *
 * alpha := eps^2
 * (C'C + alpha) * x = C'D + alpha * x0
 */
template <class TData, size_t m, size_t n>
FixedArray<TData, m, n> solve_symm(
    const FixedArray<TData, m, m>& A,
    const FixedArray<TData, m, n>& B)
{
    FixedArray<TData, m, m> L = cholesky(A);
    return solve_LU(L, L.H(), B);
}

template <class TData, size_t m>
FixedArray<TData, m> solve_symm_1d(
    const FixedArray<TData, m, m>& A,
    const FixedArray<TData, m>& B)
{
    auto res = solve_symm(A, B.reshaped(FixedArrayShape<m, 1>()));
    return res.reshaped(FixedArrayShape<m>());
}

}
