#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>

namespace Mlib {

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

}
