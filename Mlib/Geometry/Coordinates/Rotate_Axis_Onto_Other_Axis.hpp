#pragma once
#include <Mlib/Math/Fixed_Rodrigues.hpp>

namespace Mlib {

/** Rotate a matrix s.t. a specific axis is on another axis afterwards.
 */
template <class TData>
FixedArray<TData, 3, 3> rotate_axis_onto_other_axis(
    const FixedArray<TData, 3, 3>& R,
    const FixedArray<TData, 3>& abs_fixed_axis,
    const FixedArray<TData, 3>& rel_rotating_axis,
    float relaxation = 1)
{
    FixedArray<TData, 3> g = dot(abs_fixed_axis, R);
    // Find the axis that can rotate the "abs_fixed_axis" vector onto the "abs_fixed_axis".
    FixedArray<TData, 3> d = cross(rel_rotating_axis, g);
    TData d_len2 = sum(squared(d));
    if (d_len2 > 1e-12) {
        TData d_len = std::sqrt(d_len2);
        d /= d_len;
        TData ang = relaxation * std::asin(std::clamp(d_len, 0.f, 1.f));
        return dot2d(R, rodrigues2(d, ang));
    } else {
        // Abort if we are already aligned to the target axis.
        return R;
    }
}

}
