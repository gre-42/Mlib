#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TData>
FixedArray<TData, 2> line_normal(
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1,
    bool normalize)
{
    FixedArray<TData, 2> n = {l1(1) - l0(1), l0(0) - l1(0)};
    if (normalize) {
        TData len = std::sqrt(sum(squared(n)));
        if (len < 1e-12) {
            THROW_OR_ABORT("Could not calculate distance point to line, len=" + std::to_string(len));
        }
        n /= len;
    }
    return n;
}

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
TData distance_point_to_line(
    const FixedArray<TData, 2>& p,
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1,
    bool normalize)
{
    FixedArray<float, 2> n = line_normal(l0, l1, normalize);
    return dot0d(p - l0, n);
}

template <class TData>
FixedArray<TData, 2> transform_to_line_coordinates(
    const FixedArray<TData, 2>& p,
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1,
    bool compute_center = false)
{
    FixedArray<TData, 2> d = l1 - l0;
    // Maybe merge with "intersect_lines"?
    TData dist = std::sqrt(sum(squared(d)));
    if (dist < 1e-12) {
        THROW_OR_ABORT("Could not transform to line coordinates, dist=" + std::to_string(dist));
    }
    d /= dist;
    FixedArray<TData, 2> n = {d(1), -d(0)};
    if (compute_center) {
        FixedArray<TData, 2> c = (l0 + l1) / (TData)2;
        return FixedArray<TData, 2>{
            dot0d(p - c, d) / ((TData)0.5 * dist),
            dot0d(p - c, n)};
    } else {
        return FixedArray<TData, 2>{
            dot0d(p - l0, d) / dist,
            dot0d(p - l0, n)};
    }
}

template <class TData>
void distance_point_to_line(
    const FixedArray<TData, 2>& pt,
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1,
    FixedArray<TData, 2>& dir,
    TData& distance)
{
    FixedArray<TData, 2> dl = transform_to_line_coordinates(pt, l0, l1);
    if ((dl(0) >= 0) && (dl(0) <= 1)) {
        dir = sign(dl(1)) * line_normal(l0, l1, true);  // true = normalize
        distance = std::abs(dl(1));
        return;
    }
    if (dl(0) < 0) {
        dir = pt - l0;
    } else if (dl(0) > 1) {
        dir = pt - l1;
    } else {
        THROW_OR_ABORT("distance_point_to_line internal error");
    }
    distance = std::sqrt(sum(squared(dir)));
    if (distance < 1e-12) {
        dir = 0;
        distance = 0;
        return;
    }
    dir /= distance;
}

template <class TDir, class TPos, size_t tndim>
void closest_point_to_line(
    const FixedArray<TPos, tndim>& pt,
    const RaySegment3D<TDir, TPos>& ray,
    TPos& l,
    FixedArray<TPos, tndim>& closest_point)
{
    using I = funpack_t<TPos>;
    auto r = ray.template casted<I, I>();
    l = dot0d(pt - ray.start, r.direction);
    closest_point = ray.start + r.direction * std::clamp<I>(l, 0, ray.length);
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
