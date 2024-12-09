#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <optional>

namespace Mlib {

/*
 * http://rosettacode.org/wiki/Cholesky_decomposition#Python
 */
template <class TData, size_t tsize>
std::optional<FixedArray<TData, tsize, tsize>> cholesky(
    const FixedArray<TData, tsize, tsize>& A,
    const TData& diag2_min = TData(0))
{
    FixedArray<TData, tsize, tsize> L = uninitialized;
    for (size_t i = 0; i < tsize; ++i) {
        for (size_t j = 0; j <= i; ++j) {
            TData s = 0;
            for (size_t k = 0; k < j; ++k) {
                s += L(i, k) * conju(L(j, k));
            }
            if (i == j) {
                auto diag2 = std::real(A(i, i)) - std::real(s);
                if ((diag2_min != 0) && (diag2 < diag2_min)) {
                    return std::nullopt;
                }
                L(i, j) = std::sqrt(diag2);
            } else {
                L(i, j) = (A(i, j) - s) / L(j, j);
            }
        }
    }
    return L;
}

/*
 * http://en.wikipedia.org/wiki/Triangular_matrix
 * backward substitution
 */
template <class TData, size_t tsize, size_t tsize_b>
FixedArray<TData, tsize, tsize_b> solve_LU(
    const FixedArray<TData, tsize, tsize>& L,
    const FixedArray<TData, tsize, tsize>& U,
    const FixedArray<TData, tsize, tsize_b>& B)
{
    // Ax = b -> LUx = b. Then y is defined to be Ux
    FixedArray<TData, tsize, tsize_b> x = uninitialized;
    FixedArray<TData, tsize, tsize_b> y = uninitialized;
    for (size_t v = 0; v < tsize_b; ++v) {
        // Forward solve Ly = b
        for (size_t i = 0; i < tsize; ++i) {
            y(i, v) = B(i, v);
            for (size_t j = 0; j < i; ++j) {
                y(i, v) -= L(i, j) * y(j, v);
            }
            y(i, v) /= L(i, i);
        }
        // Backward solve Ux = y
        for (size_t i = tsize - 1; i != SIZE_MAX; --i) {
            x(i, v) = y(i, v);
            for (size_t j = i + 1; j < tsize; ++j) {
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
template <class TData, size_t tsize, size_t tsize_b>
std::optional<FixedArray<TData, tsize, tsize_b>> solve_symm_inplace(
    FixedArray<TData, tsize, tsize>& A,
    FixedArray<TData, tsize, tsize_b>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const FixedArray<TData, tsize, tsize_b>* x0 = nullptr,
    const TData& diag2_min = TData(0))
{
    if (alpha != TData(0) ||
        beta != TData(0))
    {
        // OpenCV Levenberg-Marquardt
        for (size_t r = 0; r < tsize; ++r) {
            TData dr = alpha + beta * A(r, r);
            A(r, r) += dr;
            if (x0 != nullptr) {
                for (size_t c = 0; c < tsize_b; ++c) {
                    B(r, c) += (*x0)(r, c) * dr;
                }
            }
        }
    }
    std::optional<FixedArray<TData, tsize, tsize>> L = cholesky(A, diag2_min);
    if (!L.has_value()) {
        return std::nullopt;
    }
    return solve_LU(L.value(), L.value().H(), B);
}

template <class TData, size_t tsize, size_t tsize_b>
std::optional<FixedArray<TData, tsize, tsize_b>> solve_symm(
    const FixedArray<TData, tsize, tsize>& A,
    const FixedArray<TData, tsize, tsize_b>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const FixedArray<TData, tsize, tsize_b>* x0 = nullptr,
    const TData& diag2_min = TData(0))
{
    FixedArray<TData, tsize, tsize> AI{A};
    FixedArray<TData, tsize, tsize_b> BI{B};
    return solve_symm_inplace(AI, BI, alpha, beta, x0, diag2_min);
}

template <class TData, size_t tsize>
std::optional<FixedArray<TData, tsize>> solve_symm_1d(
    const FixedArray<TData, tsize, tsize>& A,
    const FixedArray<TData, tsize>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const FixedArray<TData, tsize>* x0 = nullptr,
    const TData& diag2_min = TData(0))
{
    auto res = solve_symm(
        A,
        B->as_column_vector(),
        alpha,
        beta,
        x0 != nullptr
            ? rvalue_address(x0->as_column_vector())
            : nullptr,
        diag2_min);
    if (!res.has_value()) {
        return std::nullopt;
    }
    return res.value().flattened();
}

template <class TData, size_t tsize_r, size_t tsize_ac, size_t tsize_bc>
std::optional<FixedArray<TData, tsize_ac, tsize_bc>> lstsq_chol(
    const FixedArray<TData, tsize_r, tsize_ac>& A,
    const FixedArray<TData, tsize_r, tsize_bc>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const FixedArray<TData, tsize_r, tsize_r>* dAT_A = nullptr,
    const FixedArray<TData, tsize_r, tsize_bc>* dAT_B = nullptr,
    const TData& diag2_min = TData(0))
{
    FixedArray<TData, tsize_r, tsize_ac> AT_A = dot2d(A.H(), A);
    FixedArray<TData, tsize_r, tsize_bc> AT_B = dot2d(A.H(), B);
    if (dAT_A != nullptr) {
        AT_A += *dAT_A;
    }
    if (dAT_B != nullptr) {
        AT_B += *dAT_B;
    }
    return solve_symm_inplace<TData, tsize_ac, tsize_bc>(AT_A, AT_B, alpha, beta, nullptr, diag2_min);
}

template <class TData, size_t tsize_r, size_t tsize_ac>
std::optional<FixedArray<TData, tsize_ac>> lstsq_chol_1d(
    const FixedArray<TData, tsize_r, tsize_ac>& A,
    const FixedArray<TData, tsize_r>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const FixedArray<TData, tsize_r, tsize_r>* dAT_A = nullptr,
    const FixedArray<TData, tsize_r>* dAT_b = nullptr,
    const TData& diag2_min = TData(0))
{
    const FixedArray<TData, tsize_r, 1>* dAT_B1;
    if (dAT_b != nullptr) {
        dAT_B1 = &dAT_b->as_column_vector();
    }
    auto res = lstsq_chol(
        A,
        B.as_column_vector(),
        alpha,
        beta,
        dAT_A,
        dAT_b == nullptr ? nullptr : dAT_B1,
        diag2_min);
    if (!res.has_value()) {
        return std::nullopt;
    }
    return std::optional{res.value().flattened()};
}

template <size_t r, class TArray>
void lstsq_chol_1d(
    std::optional<FixedArray<typename TArray::value_type, r>>& result,
    const TArray& A,
    const Array<typename TArray::value_type>& B,
    const typename TArray::value_type& alpha = 0,
    const typename TArray::value_type& beta = 0,
    const FixedArray<typename TArray::value_type, r, r>* dAT_A = nullptr,
    const FixedArray<typename TArray::value_type, r>* dAT_b = nullptr,
    const typename TArray::value_type& diag2_min = typename TArray::value_type(0))
{
    assert(B.ndim() == 1);
    FixedArray<typename TArray::value_type, r, r> AT_A = uninitialized;
    FixedArray<typename TArray::value_type, r, 1> AT_B = uninitialized;
    dot2d(A.vH(), A, AT_A);
    dot2d(A.vH(), B.as_column_vector(), AT_B);
    if (dAT_A != nullptr) {
        AT_A += *dAT_A;
    }
    if (dAT_b != nullptr) {
        AT_B += dAT_b->template reshaped<r, 1>();
    }
    auto res = solve_symm_inplace(AT_A, AT_B, alpha, beta, (const FixedArray<typename TArray::value_type, r, 1>*)nullptr, diag2_min);
    if (!res.has_value()) {
        result = std::nullopt;
    } else {
        result = res.value().template reshaped<r>();
    }
}

template <class TData, size_t tsize>
std::optional<FixedArray<TData, tsize, tsize>> inv(
    const FixedArray<TData, tsize, tsize>& a,
    const TData& diag2_min = TData(0))
{
    return lstsq_chol<TData, tsize, tsize, tsize>(
        a,
        fixed_identity_array<TData, tsize>(),
        0,
        0,
        nullptr,
        nullptr,
        diag2_min);
}

}
