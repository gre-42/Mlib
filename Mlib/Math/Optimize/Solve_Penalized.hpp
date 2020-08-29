#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

/*
 * Solves the equation A⁻¹B for a symmetric matrix A using Cholesky
 * decomposition.
 * [C    ] * x = [D     ]
 * [eps*I]       [eps*x0]
 *
 * alpha := eps^2
 * (C'C + alpha) * x = C'D + alpha * x0
 */
template <class TSolve, class TDerivedB, class TData>
Array<TData> solve_penalized_inplace(
    const TSolve& solve,
    Array<TData>& A,
    BaseDenseArray<TDerivedB, TData>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const Array<TData>* x0 = nullptr)
{
    assert(A->ndim() == 2);
    assert(A->shape(0) == A->shape(1));
    assert(B->ndim() == 2);
    assert(B->shape(0) == A->shape(1));
    if (x0 != nullptr) {
        assert(x0->ndim() == 2);
        assert(all(x0->shape() == B->shape()));
    }
    if (alpha != TData(0) ||
        beta != TData(0))
    {
        // OpenCV Levenberg-Marquardt
        for(size_t r = 0; r < A->shape(0); ++r) {
            TData dr = alpha + beta * A(r, r);
            A(r, r) += dr;
            if (x0 != nullptr) {
                for(size_t c = 0; c < B->shape(1); ++c) {
                    (*B)(r, c) += (*x0)(r, c) * dr;
                }
            }
        }
    }
    return solve(A, *B);
}

template <class TSolve, class TDerivedB, class TData>
Array<TData> solve_penalized(
    const TSolve& solve,
    const Array<TData>& A,
    const BaseDenseArray<TDerivedB, TData>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const Array<TData>* x0 = nullptr)
{
    Array<TData> AI{A};
    TDerivedB BI{*B};
    if (alpha != TData(0) ||
        beta != TData(0))
    {
        AI.reassign(A);
        if (x0 != nullptr) {
            BI.reassign(*B);
        }
    }
    return solve_penalized_inplace(solve, AI, BI, alpha, beta, x0);
}

}
