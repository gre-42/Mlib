#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O3")
#endif

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

}

#ifdef __GNUC__
#pragma GCC pop_options
#endif
