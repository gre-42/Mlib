#pragma once
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

}
