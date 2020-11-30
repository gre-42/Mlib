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
TData distance_point_to_triangle (
    const FixedArray<TData, 2>& pt,
    const FixedArray<TData, 2>& v1,
    const FixedArray<TData, 2>& v2,
    const FixedArray<TData, 2>& v3)
{
    TData dl1 = distance_point_to_line(pt, v1, v2, true);
    TData dl2 = distance_point_to_line(pt, v2, v3, true);
    TData dl3 = distance_point_to_line(pt, v3, v1, true);
    TData dl = std::max(std::max(dl1, dl2), dl3);

    if (dl <= 0) {
        return 0;
    }

    TData sp1 = sum(squared(v1 - pt));
    TData sp2 = sum(squared(v2 - pt));
    TData sp3 = sum(squared(v3 - pt));
    TData sp = std::min(std::min(sp1, sp2), sp3);

    if (squared(dl) >= sp) {
        return dl;
    } else {
        return std::sqrt(sp);
    }
}

}

#pragma GCC pop_options
