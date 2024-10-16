#pragma once
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
TData quad_area(
    const FixedArray<TData, 3>& q0,
    const FixedArray<TData, 3>& q1,
    const FixedArray<TData, 3>& q2,
    const FixedArray<TData, 3>& q3)
{
    // https://en.wikipedia.org/wiki/Quadrilateral#Area_of_a_convex_quadrilateral
    return std::sqrt(sum(squared(cross(q0 - q2, q1 - q3)))) / 2;
}

template <class TData>
TData quad_area(
    const FixedArray<TData, 2>& q0,
    const FixedArray<TData, 2>& q1,
    const FixedArray<TData, 2>& q2,
    const FixedArray<TData, 2>& q3)
{
    FixedArray<TData, 2, 2> m = uninitialized;
    m[0] = q2 - q0;
    m[1] = q3 - q1;
    return det2x2(m) / 2;
}

}
