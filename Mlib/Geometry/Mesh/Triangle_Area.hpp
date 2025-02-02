#pragma once
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
TData triangle_area(
    const FixedArray<TData, 3>& t0,
    const FixedArray<TData, 3>& t1,
    const FixedArray<TData, 3>& t2)
{
    // https://math.stackexchange.com/questions/128991/how-to-calculate-the-area-of-a-3d-triangle
    return std::sqrt(sum(squared(cross(t0 - t1, t0 - t2)))) / 2;
}

template <class TData>
TData triangle_area(
    const FixedArray<TData, 2>& t0,
    const FixedArray<TData, 2>& t1,
    const FixedArray<TData, 2>& t2)
{
    FixedArray<TData, 2, 2> m = uninitialized;
    m[0] = t1 - t0;
    m[1] = t2 - t0;
    return det2x2(m) / 2;
}

template <class TData, size_t tndim>
TData triangle_area(const FixedArray<TData, 3, tndim>& triangle) {
    return triangle_area(triangle[0], triangle[1], triangle[2]);
}

}
