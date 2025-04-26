#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TPos, class TWidth>
FixedArray<TPos, 3> point_to_grid_center(
    const FixedArray<TPos, 3>& position,
    const FixedArray<TWidth, 3>& width)
{
    return (round(funpack(position) / width) * width).template casted<TPos>();
}

}
