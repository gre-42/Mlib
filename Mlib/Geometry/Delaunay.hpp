#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>

namespace Mlib {

enum class DelaunayState {
    DELAUNAY,
    NOT_DELAUNAY,
    ERROR
};

/* From: https://en.wikipedia.org/wiki/Delaunay_triangulation#Algorithms
 */
template <class TPos>
bool is_delaunay(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    const FixedArray<TPos, 2>& d)
{
    auto ad = a - d;
    auto bd = b - d;
    auto cd = c - d;
    auto ad2 = squared(a) - squared(d);
    auto bd2 = squared(b) - squared(d);
    auto cd2 = squared(c) - squared(d);
    return det3x3(FixedArray<TPos, 3, 3>::init(
        ad(0), ad(1), ad2(0) + ad(1),
        bd(0), bd(1), bd2(0) + bd(1),
        cd(0), cd(1), cd2(0) + cd(1))) <= 0;
}

template <class TPos>
DelaunayState is_delaunay(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    const FixedArray<TPos, 3>& d)
{
    auto n = cross(funpack(b - a), funpack(c - a));
    auto l_n = std::sqrt(sum(squared(n)));
    if (l_n < 1e-12) {
        return DelaunayState::ERROR;
    }
    n /= l_n;
    auto x = funpack(b - a);
    auto l_x = std::sqrt(sum(squared(x)));
    if (l_x < 1e-12) {
        return DelaunayState::ERROR;
    }
    x /= l_x;
    auto y = cross(n, x);
    auto m = FixedArray<funpack_t<TPos>, 2, 3>{x, y};
    auto ra = dot1d(m, funpack(a - a));
    auto rb = dot1d(m, funpack(b - a));
    auto rc = dot1d(m, funpack(c - a));
    auto rd = dot1d(m, funpack(d - a));
    return is_delaunay(ra, rb, rc, rd)
        ? DelaunayState::DELAUNAY
        : DelaunayState::NOT_DELAUNAY;
}

}
