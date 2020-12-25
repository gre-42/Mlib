#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

/*
 * Outer product of two matrices.
 * outer2d(a, b) = dot(a, b.H())
 */
template <class TData, size_t nrows, size_t ncolumns, size_t nsum>
FixedArray<TData, nrows, ncolumns> outer2d(
    const FixedArray<TData, nrows, nsum>& a,
    const FixedArray<TData, ncolumns, nsum>& b)
{
    FixedArray<TData, nrows, ncolumns> result;
    for (size_t r = 0; r < nrows; ++r) {
        for (size_t c = 0; c < ncolumns; ++c) {
            TData v = 0;
            for (size_t i = 0; i < nsum; ++i) {
                v += a(r, i) * conju(b(c, i));
            }
            result(r, c) = v;
        }
    }
    return result;
}

/*
 * Outer product of two matrices (sum over last axes).
 * ND-wrapper around outer2d.
 * 2D: outer(a, b) = dot(a, b.H())
 */
template <class TData, size_t... tsize_a, size_t... tsize_b>
auto outer(
    const FixedArray<TData, tsize_a...>& a,
    const FixedArray<TData, tsize_b...>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (b_l0, b_l1, n)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    if constexpr ((a.ndim() != 2) || (b.ndim() != 2)) {
        const auto& a2 = a.rows_as_1D();
        const auto& b2 = b.rows_as_1D();
        auto r = outer2d(a2, b2);
        auto r_shape =
            a.shape()
            .erased_last()
            .concatenated(
                b.shape()
                .erased_last());
        return r.reshaped(r_shape);
    } else {
        return outer2d(a, b);
    }
}

template <class TData, size_t nrows, size_t ncolumns, size_t nsum>
FixedArray<TData, nrows, ncolumns> dot2d(
    const FixedArray<TData, nrows, nsum>& a,
    const FixedArray<TData, nsum, ncolumns>& b)
{
    FixedArray<TData, nrows, ncolumns> result;
    for (size_t r = 0; r < nrows; ++r) {
        for (size_t c = 0; c < ncolumns; ++c) {
            TData v = 0;
            for (size_t i = 0; i < nsum; ++i) {
                v += a(r, i) * b(i, c);
            }
            result(r, c) = v;
        }
    }
    return result;
}

template <class TData, size_t... tsize_a, size_t... tsize_b>
auto dot(
    const FixedArray<TData, tsize_a...>& a,
    const FixedArray<TData, tsize_b...>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (n, b_l0, b_l1)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    if constexpr ((a.ndim() != 2) || (b.ndim() != 2)) {
        const auto& a2 = a.rows_as_1D();
        const auto& b2 = b.columns_as_1D();
        auto r = dot2d(a2, b2);
        auto r_shape =
            a.shape()
            .erased_last()
            .concatenated(
                b.shape()
                .erased_first());
        return r.reshaped(r_shape);
    } else {
        return dot2d(a, b);
    }
}

template <class TData, size_t tsize_r, size_t tsize_c>
auto dot1d(
    const FixedArray<TData, tsize_r, tsize_c>& a,
    const FixedArray<TData, tsize_c>& b)
{
    return dot(a, b);
}

template <class TData, size_t tsize>
TData dot0d(
    const FixedArray<TData, tsize>& a,
    const FixedArray<TData, tsize>& b)
{
    return dot(a, b)();
}

template <class TData, size_t tshape0, size_t... tshape>
void assert_allequal(const FixedArray<TData, tshape0, tshape...>& a, const FixedArray<TData, tshape0, tshape...>& b) {
    for (size_t i = 0; i < tshape0; ++i) {
        assert_allequal(a[i], b[i]);
    }
}

template <class TData>
void assert_allequal(const FixedArray<TData>& a, const FixedArray<TData>& b) {
    if (!(a() == b()) && !(std::isnan(a()) && std::isnan(b()))) {
        throw std::runtime_error("Numbers not identical: " + std::to_string(a()) + ", " + std::to_string(b()));
    }
}

template <class TData, size_t n>
FixedArray<TData, n, n> fixed_identity_array() {
    FixedArray<TData, n, n> result;
    for (size_t r = 0; r < n; ++r){
        for (size_t c = 0; c < n; ++c) {
            result(r, c) = (r == c);
        }
    }
    return result;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...> fixed_full(const TData& value) {
    FixedArray<TData, tsize...> a;
    a = value;
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...> fixed_zeros() {
    return fixed_full<TData, tsize...>(0);
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...> fixed_ones() {
    return fixed_full<TData, tsize...>(1);
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...> fixed_nans() {
    return fixed_full<TData, tsize...>(NAN);
}

}
