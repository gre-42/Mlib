#pragma once
#include <Mlib/Array/Array_Forward.hpp>

#pragma GCC push_options
#pragma GCC optimize ("O3")

namespace Mlib {

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
TData point_line_sign(
    const FixedArray<TData, 2>& p1,
    const FixedArray<TData, 2>& p2,
    const FixedArray<TData, 2>& p3)
{
    return (p1(0) - p3(0)) * (p2(1) - p3(1)) - (p2(0) - p3(0)) * (p1(1) - p3(1));
}

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
bool point_is_in_triangle (
    const FixedArray<TData, 2>& pt,
    const FixedArray<TData, 2>& v1,
    const FixedArray<TData, 2>& v2,
    const FixedArray<TData, 2>& v3)
{
    TData d1, d2, d3;
    bool has_neg, has_pos;

    d1 = point_line_sign(pt, v1, v2);
    d2 = point_line_sign(pt, v2, v3);
    d3 = point_line_sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

}

#pragma GCC pop_options
