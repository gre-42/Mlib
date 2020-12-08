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
    bool is_inside = true;
    TData result_edge = NAN;
    auto include_edge = [&pt, &is_inside, &result_edge](
        const FixedArray<TData, 2>& a,
        const FixedArray<TData, 2>& b)
    {
        FixedArray<TData, 2> dl = transform_to_line_coordinates(pt, a, b);
        if (dl(1) >= 0) {
            is_inside = false;
            if ((dl(0) >= 0) && (dl(0) <= 1)) {
                if (!std::isnan(result_edge)) {
                    throw std::runtime_error("distance_point_to_triangle detected left-handed triangle");
                }
                result_edge = dl(1);
            }
        }
    };
    include_edge(v1, v2);
    include_edge(v2, v3);
    include_edge(v3, v1);
    if (is_inside) {
        return 0;
    } else if (!std::isnan(result_edge)) {
        return result_edge;
    } else {
        TData sp1 = sum(squared(v1 - pt));
        TData sp2 = sum(squared(v2 - pt));
        TData sp3 = sum(squared(v3 - pt));
        return std::sqrt(std::min(std::min(sp1, sp2), sp3));
    }
}

}

#pragma GCC pop_options
