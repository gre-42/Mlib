#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> conju_x(const Array<TData>& a) {
    return a.template applied([](const TData& v){return conju(v);});
}

template <class TData>
Array<Array<TData>> vH_x(const Array<Array<TData>>& a) {
    Array<Array<TData>> res = a.T();
    for (Array<TData>& x : res.flat_iterable()) {
        x = x.template applied([](const TData& v){return conju(v);});
    }
    return res;
}

template <class TDerivedA, class TDerivedB, class TDerivedR, class TData>
void batch_dot2d(
    const BaseDenseArray<TDerivedA, Array<TData>>& a,
    const BaseDenseArray<TDerivedB, Array<TData>>& b,
    BaseDenseArray<TDerivedR, Array<TData>>& result)
{
    assert(a->ndim() == 2);
    assert(b->ndim() == 2);
    assert(a->shape(1) == b->shape(0));
    assert(all(result->shape() == ArrayShape{a->shape(0), b->shape(1)}));
    #pragma omp parallel for if (result->nelements() > 200 * 200)
    for (size_t r = 0; r < result->shape(0); ++r) {
        for (size_t c = 0; c < result->shape(1); ++c) {
            Array<TData> v = zeros<TData>((*a)(0, 0).shape());
            for (size_t i = 0; i < a->shape(1); ++i) {
                v += (*a)(r, i) * (*b)(i, c);
            }
            (*result)(r, c) = v;
        }
    }
}

template <class TDerivedA, class TDerivedB, class TData>
Array<Array<TData>> batch_dot2d(
    const BaseDenseArray<TDerivedA, Array<TData>>& a,
    const BaseDenseArray<TDerivedB, Array<TData>>& b)
{
    assert(a->ndim() == 2);
    assert(b->ndim() == 2);
    assert(a->shape(1) == b->shape(0));
    Array<Array<TData>> result{ArrayShape{a->shape(0), b->shape(1)}};
    batch_dot2d(a, b, result);
    return result;
}

template <class TDerivedA, class TDerivedB, class TData>
Array<Array<TData>> batch_dot1d(
    const BaseDenseArray<TDerivedA, Array<TData>>& a,
    const BaseDenseArray<TDerivedB, Array<TData>>& b)
{
    assert(a->ndim() == 2);
    assert(b->ndim() == 1);
    assert(a->shape(1) == b->length());
    Array<Array<TData>> result{ArrayShape{a->shape(0), 1}};
    batch_dot2d(a, b->reshaped(ArrayShape{b->length(), 1}), result);
    return result.flattened();
}

/*
 * http://rosettacode.org/wiki/Cholesky_decomposition#Python
 */
template <class TData>
Array<Array<TData>> batch_cholesky(const Array<Array<TData>>& A) {
    assert(A.shape(0) == A.shape(1));
    Array<Array<TData>> L;
    L.resize(A.shape());
    for (size_t i = 0; i < A.shape(0); ++i) {
        for (size_t j = 0; j <= i; ++j) {
            Array<TData> s = zeros<TData>(L(0, 0).shape());
            for (size_t k = 0; k < j; ++k) {
                s += L(i, k) * conju_x(L(j, k));
            }
            L(i, j) = (i == j) ? sqrt(A(i, i) - s) : ((A(i, j) - s) / L(j, j));
        }
    }
    return L;
}

/*
 * http://en.wikipedia.org/wiki/Triangular_matrix
 * backward substitution
 */
template <class TDerivedL, class TDerivedU, class TDerivedB, class TData>
Array<TData> batch_solve_LU(
    const BaseDenseArray<TDerivedL, TData>& L,
    const BaseDenseArray<TDerivedU, TData>& U,
    const BaseDenseArray<TDerivedB, TData>& B)
{
    // Ax = b -> LUx = b. Then y is defined to be Ux
    assert(L->ndim() >= 2);
    assert(U->ndim() >= 2);
    assert(B->ndim() >= 2);
    assert(L->shape(0) == L->shape(1)); // square
    assert(all(L->shape() == U->shape()));
    assert(B->shape(0) == U->shape(1));
    Array<TData> x, y;
    x.resize(B->shape());
    y.resize(B->shape());
    for (size_t v = 0; v < B->shape(1); ++v) {
        // Forward solve Ly = b
        for (size_t i = 0; i < B->shape(0); ++i) {
            y(i, v) = (*B)(i, v);
            for (size_t j = 0; j < i; ++j) {
                y(i, v) -= (*L)(i, j) * y(j, v);
            }
            y(i, v) /= (*L)(i, i);
        }
        // Backward solve Ux = y
        for (size_t i = B->shape(0) - 1; i != SIZE_MAX; --i) {
            x(i, v) = y(i, v);
            for (size_t j = i + 1; j < B->shape(0); ++j) {
                x(i, v) -= (*U)(i, j) * x(j, v);
            }
            x(i, v) /= (*U)(i, i);
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
template <class TDerivedB, class TData>
Array<Array<TData>> batch_solve_symm_inplace(
    Array<Array<TData>>& A,
    BaseDenseArray<TDerivedB, Array<TData>>& B,
    const Array<TData>* alpha = nullptr,
    const Array<TData>* beta = nullptr,
    const Array<Array<TData>>* x0 = nullptr)
{
    assert(A->ndim() == 2);
    assert(A->shape(0) == A->shape(1));
    assert(B->ndim() == 2);
    assert(B->shape(0) == A->shape(1));
    if (x0 != nullptr) {
        assert(x0->ndim() == 2);
        assert(all(x0->shape() == B->shape()));
    }
    if (alpha != nullptr || beta != nullptr) {
        // OpenCV Levenberg-Marquardt
        for (size_t r = 0; r < A->shape(0); ++r) {
            Array<TData> dr = (*alpha) + (*beta) * A(r, r);
            A(r, r) += dr;
            if (x0 != nullptr) {
                for (size_t c = 0; c < B->shape(1); ++c) {
                    (*B)(r, c) += (*x0)(r, c) * dr;
                }
            }
        }
    }
    Array<Array<TData>> L = batch_cholesky(A);
    return batch_solve_LU(L, vH_x(L), B);
}

template <class TDerivedB, class TData>
Array<Array<TData>> batch_solve_symm(
    const Array<Array<TData>>& A,
    const BaseDenseArray<TDerivedB, Array<TData>>& B,
    const Array<TData>* alpha = nullptr,
    const Array<TData>* beta = nullptr,
    const Array<Array<TData>>* x0 = nullptr)
{
    Array<Array<TData>> AI{A};
    TDerivedB BI{*B};
    if (alpha != nullptr || beta != nullptr) {
        AI.reassign(A);
        if (x0 != nullptr) {
            BI.reassign(*B);
        }
    }
    return batch_solve_symm_inplace(AI, BI, alpha, beta, x0);
}

template <class TDerivedB, class TData>
Array<Array<TData>> batch_solve_symm_1d(
    const Array<Array<TData>>& A,
    const BaseDenseArray<TDerivedB, Array<TData>>& B,
    const Array<TData>* alpha = nullptr,
    const Array<TData>* beta = nullptr,
    const Array<Array<TData>>* x0 = nullptr)
{
    assert(B->ndim() == 1);
    if (x0 != nullptr) {
        assert(x0->length() == A.shape(0));
    }
    auto res = batch_solve_symm(
        A,
        B->as_column_vector(),
        alpha,
        beta,
        x0 != nullptr
            ? rvalue_address(x0->as_column_vector())
            : nullptr);
    return res.flattened();
}

}
