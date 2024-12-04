#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <concepts>

namespace Mlib {

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator += (
    FixedArray<TData, tsize...>& a,
    const FixedArray<TData, tsize...>& b)
{
    auto v0 = a.flat_begin();
    auto v1 = b.flat_begin();
    while(v0 != a.flat_end()) {
        *v0++ += *v1++;
    }
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator -= (
    FixedArray<TData, tsize...>& a,
    const FixedArray<TData, tsize...>& b)
{
    auto v0 = a.flat_begin();
    auto v1 = b.flat_begin();
    while( v0 != a.flat_end()) {
        *v0++ -= *v1++;
    }
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator *= (
    FixedArray<TData, tsize...>& a,
    const FixedArray<TData, tsize...>& b)
{
    auto v0 = a.flat_begin();
    auto v1 = b.flat_begin();
    while (v0 != a.flat_end()) {
        *v0++ *= *v1++;
    }
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator /= (
    FixedArray<TData, tsize...>& a,
    const FixedArray<TData, tsize...>& b)
{
    auto v0 = a.flat_begin();
    auto v1 = b.flat_begin();
    while (v0 != a.flat_end()) {
        *v0++ /= *v1++;
    }
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator *= (
    FixedArray<TData, tsize...>& a,
    const TData& b)
{
    for (TData& v : a.flat_iterable())
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        v *= b;
#pragma GCC diagnostic pop
    }
    return a;
}

template <class TData, size_t... tsize>
FixedArray<TData, tsize...>& operator /= (
    FixedArray<TData, tsize...>& a,
    const TData& b)
{
    for (TData& v : a.flat_iterable()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        v /= b;
#pragma GCC diagnostic pop
    }
    return a;
}

template <class TData, size_t... tsize, std::integral I>
FixedArray<TData, tsize...>& operator /= (
    FixedArray<TData, tsize...>& a,
    I b)
{
    for (TData& v : a.flat_iterable()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        v /= b;
#pragma GCC diagnostic pop
    }
    return a;
}

/*
 * Outer product of two matrices.
 * outer2d(a, b) = dot(a, b.H())
 */
template <class TData, size_t nrows, size_t ncolumns, size_t nsum>
constexpr FixedArray<TData, nrows, ncolumns> outer2d(
    const FixedArray<TData, nrows, nsum>& a,
    const FixedArray<TData, ncolumns, nsum>& b)
{
    FixedArray<TData, nrows, ncolumns> result = uninitialized;
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
constexpr auto outer(
    const FixedArray<TData, tsize_a...>& a,
    const FixedArray<TData, tsize_b...>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (b_l0, b_l1, n)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    if constexpr (
        (FixedArray<TData, tsize_a...>::ndim() != 2) ||
        (FixedArray<TData, tsize_b...>::ndim() != 2))
    {
        const auto& a2 = a.rows_as_1D();
        const auto& b2 = b.rows_as_1D();
        auto r = outer2d(a2, b2);
        auto r_shape =
            std::remove_reference_t<decltype(a)>::shape()
            .erased_last()
            .concatenated(
                std::remove_reference_t<decltype(b)>::shape()
                .erased_last());
        return r.reshaped(r_shape);
    } else {
        return outer2d(a, b);
    }
}

template <class TDerived, class TData, size_t nrows, size_t ncolumns, size_t nsum>
constexpr FixedArray<TData, nrows, ncolumns> dot2d(
    const BaseDenseFixedArray<TDerived, TData, nrows, nsum>& a,
    const FixedArray<TData, nsum, ncolumns>& b)
{
    FixedArray<TData, nrows, ncolumns> result = uninitialized;
    for (size_t r = 0; r < nrows; ++r) {
        for (size_t c = 0; c < ncolumns; ++c) {
            TData v = 0;
            for (size_t i = 0; i < nsum; ++i) {
                v += (*a)(r, i) * b(i, c);
            }
            result(r, c) = v;
        }
    }
    return result;
}

template <class TDerived, class TData, size_t... tsize>
constexpr const auto& rows_as_1D(const BaseDenseFixedArray<TDerived, TData, tsize...>& a) {
    if constexpr (FixedArrayShape<tsize...>::ndim() == 2) {
        return *a;
    } else {
        return a->rows_as_1D();
    }
}

template <class TDerived, class TData, size_t... tsize>
constexpr const auto& columns_as_1D(const BaseDenseFixedArray<TDerived, TData, tsize...>& a) {
    if constexpr (FixedArrayShape<tsize...>::ndim() == 2) {
        return *a;
    } else {
        return a->columns_as_1D();
    }
}

template <class TDerived, class TData, size_t... tsize_a, size_t... tsize_new>
constexpr const auto& reshaped(
    const BaseDenseFixedArray<TDerived, TData, tsize_a...>& a,
    const FixedArrayShape<tsize_new...>& new_shape) {
    if constexpr (FixedArrayShape<tsize_a...>() == FixedArrayShape<tsize_new...>()) {
        return *a;
    } else {
        return a->columns_as_1D();
    }
}

template <class TDerived, class TData, size_t... tsize_a, size_t... tsize_b>
constexpr auto dot(
    const BaseDenseFixedArray<TDerived, TData, tsize_a...>& a,
    const FixedArray<TData, tsize_b...>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (n, b_l0, b_l1)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    const auto& a2 = rows_as_1D(a);
    const auto& b2 = columns_as_1D(b);
    auto r = dot2d(a2, b2);
    auto r_shape =
        FixedArrayShape<tsize_a...>()
        .erased_last()
        .concatenated(
            FixedArrayShape<tsize_b...>()
            .erased_first());
    return r.reshaped(r_shape);
}

template <class TDerived, class TData, size_t tsize_r, size_t tsize_c>
constexpr FixedArray<TData, tsize_r> dot1d(
    const BaseDenseFixedArray<TDerived, TData, tsize_r, tsize_c>& a,
    const FixedArray<TData, tsize_c>& b)
{
    return dot(a, b);
}

template <class TData, size_t tsize>
constexpr TData dot0d(
    const FixedArray<TData, tsize>& a,
    const FixedArray<TData, tsize>& b)
{
    return dot(a, b)();
}

template <class TData, size_t n>
constexpr FixedArray<TData, n, n> fixed_identity_array() {
    FixedArray<TData, n, n> result = uninitialized;
    for (size_t r = 0; r < n; ++r){
        for (size_t c = 0; c < n; ++c) {
            result(r, c) = (r == c);
        }
    }
    return result;
}

template <class TData, size_t... tsize>
constexpr FixedArray<TData, tsize...> fixed_full(const TData& value) {
    FixedArray<TData, tsize...> a = uninitialized;
    a = value;
    return a;
}

template <class TData, size_t... tsize>
constexpr FixedArray<TData, tsize...> fixed_zeros() {
    return fixed_full<TData, tsize...>((TData)0.f);
}

template <class TData, size_t... tsize>
constexpr FixedArray<TData, tsize...> fixed_ones() {
    return fixed_full<TData, tsize...>((TData)1.f);
}

template <class TData, size_t... tsize>
constexpr FixedArray<TData, tsize...> fixed_nans() {
    return fixed_full<TData, tsize...>(NAN);
}

template <class TData, size_t ...tshape, typename... TIndices>
constexpr FixedArray<TData, tshape...> fixed_dirac_array(const TIndices&... indices) {
    FixedArray<TData, tshape...> result = fixed_zeros<TData, tshape...>();
    result(indices...) = (TData)1.f;
    return result;
}

template <size_t axis, class TData, size_t... tsize>
auto sum(const FixedArray<TData, tsize...>& a) {
    constexpr auto shape3 = decltype(a.shape())::template axis_as_3D<axis>();
    constexpr auto shape2 = decltype(a.shape())::template without_axis<axis>();
    constexpr size_t R = shape3.template get<0>();
    constexpr size_t A = shape3.template get<1>();
    constexpr size_t C = shape3.template get<2>();
    auto result = fixed_zeros<TData, R, C>();
    const auto& a3 = a.reshaped(shape3);
    for (size_t r = 0; r < R; ++r) {
        for (size_t c = 0; c < C; ++c) {
            for (size_t i = 0; i < A; ++i) {
                result(r, c) += a3(r, i, c);
            }
        }
    }
    return result.reshaped(shape2);
}

}
