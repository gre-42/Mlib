#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>

PRAGMA_GCC_O3_BEGIN

namespace Mlib {

template <class TData>
bool distance_line_line(
    const FixedArray<TData, 2, 3>& line0,
    const FixedArray<TData, 2, 3>& line1,
    TData& distance)
{
    // From: https://math.stackexchange.com/questions/2213165/find-shortest-distance-between-lines-in-3d
    auto n = cross(line0[1] - line0[0], line1[1] - line1[0]);
    auto l2 = sum(squared(n));
    if (l2 < 1e-12) {
        return false;
    }
    distance = std::abs(dot0d(n, line0[0] - line1[0])) / std::sqrt(l2);
    return true;
}

template <class TData, size_t tndim>
bool distance_line_line(
    const FixedArray<TData, 2, tndim>& line0,
    const FixedArray<TData, 2, tndim>& line1,
    FixedArray<TData, tndim>& p0,
    FixedArray<TData, tndim>& p1)
{
    // From: https://math.stackexchange.com/questions/2213165/find-shortest-distance-between-lines-in-3d
    const auto& a = line0[0];
    const auto& c = line1[0];
    auto b = line0[1] - line0[0];
    auto d = line1[1] - line1[0];
    auto e = line0[0] - line1[0];
    auto b2 = sum(squared(b));
    auto d2 = sum(squared(d));
    auto b2d2 = b2 * d2;
    auto bd = dot0d(b, d);
    auto bd2 = squared(bd);
    auto A = -b2d2 + bd2;
    if (std::abs(A) < 1e-12) {
        return false;
    }
    auto be = dot0d(b, e);
    auto de = dot0d(d, e);
    auto s = (-b2 * de + be * bd) / A;
    auto t = (d2 * be - de * bd) / A;
    if ((t < 0) || (t > 1)) {
        return false;
    }
    if ((s < 0) || (s > 1)) {
        return false;
    }
    p0 = a + b * t;
    p1 = c + d * s;
    return true;
}

template <class TDir, class TPos, size_t tndim>
bool distance_ray_ray(
    const RaySegment3D<TDir, TPos>& ray0,
    const RaySegment3D<TDir, TPos>& ray1,
    FixedArray<TPos, tndim>& p0,
    FixedArray<TPos, tndim>& p1)
{
    // From: https://math.stackexchange.com/questions/2213165/find-shortest-distance-between-lines-in-3d
    const auto& a = ray0.start;
    const auto& c = ray1.start;
    const auto& b = ray0.direction;
    const auto& d = ray1.direction;
    auto bd = dot0d(b, d);
    auto bd2 = squared(bd);
    auto A = -1 + bd2;
    if (std::abs(A) < 1e-12) {
        return false;
    }
    auto e = (ray0.start - ray1.start).template casted<TDir>();
    auto be = dot0d(b, e);
    auto de = dot0d(d, e);
    auto s = (-de + be * bd) / A;
    auto t = (be - de * bd) / A;
    if ((t < 0) || (t > ray0.length)) {
        return false;
    }
    if ((s < 0) || (s > ray1.length)) {
        return false;
    }
    p0 = a + b.template casted<TPos>() * t;
    p1 = c + d.template casted<TPos>() * s;
    return true;
}

}

PRAGMA_GCC_O3_END
