#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Intersection/Distance/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
bool point_is_in_triangle(
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
TData distance_point_to_triangle(
    const FixedArray<TData, 2>& pt,
    const FixedArray<TData, 3, 2>& v,
    FixedArray<TData, 2>* closest_point = nullptr)
{
    bool is_inside = true;
    TData result_edge = NAN;
    auto include_edge = [&](
        const FixedArray<TData, 2>& a,
        const FixedArray<TData, 2>& b)
    {
        if (sum(squared(a - b)) < 1e-12) {
            is_inside = false;
        } else {
            FixedArray<TData, 2> dl = transform_to_line_coordinates(pt, a, b);
            if (dl(1) >= 0) {
                is_inside = false;
                // Using "> 0" and "< 1" instead of ">= 0" and "<= 1" to support pt=v1 or pt=v2 or pt=v3.
                if ((dl(0) > 0) && (dl(0) < 1)) {
                    if (!std::isnan(result_edge)) {
                        THROW_OR_ABORT("distance_point_to_triangle detected left-handed triangle");
                    }
                    if (closest_point != nullptr) {
                        *closest_point = lerp(a, b, dl(0));
                    }
                    result_edge = dl(1);
                }
            }
        }
    };
    include_edge(v[0], v[1]);
    include_edge(v[1], v[2]);
    include_edge(v[2], v[0]);
    if (is_inside) {
        if (closest_point != nullptr) {
            *closest_point = pt;
        }
        return 0;
    } else if (!std::isnan(result_edge)) {
        return result_edge;
    } else {
        FixedArray<TData, 3> sp{
            sum(squared(v[0] - pt)),
            sum(squared(v[1] - pt)),
            sum(squared(v[2] - pt)) };
        size_t ib = 0;
        for (size_t i = 1; i < 3; ++i) {
            if (sp(i) < sp(ib)) {
                ib = i;
            }
        }
        if (closest_point != nullptr) {
            *closest_point = v[ib];
        }
        return std::sqrt(sp(ib));
    }
}

template <class TData>
FixedArray<TData, 2> distance_point_to_triangle_3d(
    const FixedArray<TData, 3>& pt,
    const FixedArray<TData, 3, 3>& v,
    const FixedArray<TData, 3>& normal)
{
    FixedArray<TData, 2, 3> m = uninitialized;
    m[0] = v[1] - v[0];
    m[0] /= std::sqrt(sum(squared(m[0])));
    m[1] = cross(normal, m[0]);
    TData dist0 = dot0d(pt - v[0], normal);
    TData dist1 = distance_point_to_triangle(
        dot1d(m, pt),
        outer2d(v, m));
    return { dist0, dist1 };
}

template <class TData>
FixedArray<TData, 2> distance_point_to_triangle_3d(
    const FixedArray<TData, 3>& pt,
    const FixedArray<TData, 3, 3>& v)
{
    return distance_point_to_triangle_3d(
        pt,
        v,
        triangle_normal(v));
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
