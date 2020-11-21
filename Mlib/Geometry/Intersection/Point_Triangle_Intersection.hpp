#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>

#pragma GCC push_options
#pragma GCC optimize ("O3")

namespace Mlib {

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
    TData d1 = distance_point_to_line(pt, v1, v2, false);
    TData d2 = distance_point_to_line(pt, v2, v3, false);
    TData d3 = distance_point_to_line(pt, v3, v1, false);

    return (d1 <= 0) && (d2 <= 0) && (d3 <= 0);
}

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
float distance_point_to_triangle (
    const FixedArray<TData, 2>& pt,
    const FixedArray<TData, 2>& v1,
    const FixedArray<TData, 2>& v2,
    const FixedArray<TData, 2>& v3)
{
    TData d1 = distance_point_to_line(pt, v1, v2, true);
    TData d2 = distance_point_to_line(pt, v2, v3, true);
    TData d3 = distance_point_to_line(pt, v3, v1, true);

    return std::max(std::max(d1, d2), d3);
}

}

#pragma GCC pop_options
