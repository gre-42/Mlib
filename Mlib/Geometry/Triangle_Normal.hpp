#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

inline FixedArray<float, 3> scaled_triangle_normal(const FixedArray<FixedArray<float, 3>, 3>& t)
{
    return cross(t(2) - t(1), t(0) - t(1));
}

inline FixedArray<float, 3> triangle_normal(const FixedArray<FixedArray<float, 3>, 3>& t)
{
    FixedArray<float, 3> res = scaled_triangle_normal(t);
    res /= std::sqrt(sum(squared(res)));
    return res;
}

}
