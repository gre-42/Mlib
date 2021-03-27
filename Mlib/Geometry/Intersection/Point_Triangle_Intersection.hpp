#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

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
            // Using "< 1" instead of "<= 1" to support pt=v1 or pt=v2 or pt=v3.
            if ((dl(0) >= 0) && (dl(0) < 1)) {
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
        return std::sqrt(std::min({sp1, sp2, sp3}));
    }
}

template <class TData>
TData distance_point_to_triangle_3d (
    const FixedArray<TData, 3>& pt,
    const FixedArray<TData, 3>& v1,
    const FixedArray<TData, 3>& v2,
    const FixedArray<TData, 3>& v3,
    const FixedArray<TData, 3>& normal)
{
    FixedArray<TData, 2, 3> m;
    m[0] = v2 - v1;
    m[0] /= std::sqrt(sum(squared(m[0])));
    m[1] = cross(normal, m[0]);
    TData dist0 = dot0d(pt - v1, normal);
    TData dist1 = distance_point_to_triangle(
        dot1d(m, pt),
        dot1d(m, v1),
        dot1d(m, v2),
        dot1d(m, v3));
    return std::sqrt(squared(dist0) + squared(dist1));
}

template <class TData>
TData distance_point_to_triangle_3d (
    const FixedArray<TData, 3>& pt,
    const FixedArray<TData, 3>& v1,
    const FixedArray<TData, 3>& v2,
    const FixedArray<TData, 3>& v3)
{
    return distance_point_to_triangle_3d(
        pt,
        v1,
        v2,
        v3,
        triangle_normal(FixedArray<FixedArray<TData, 3>, 3>{v1, v2, v3}));
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
