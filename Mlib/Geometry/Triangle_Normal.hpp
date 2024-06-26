#pragma once
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Triangle_Normal_Error_Behavior.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TData>
FixedArray<TData, 3> scaled_triangle_normal(const FixedArray<FixedArray<TData, 3>, 3>& t)
{
    return cross(t(2) - t(1), t(0) - t(1));
}

template <class TData>
FixedArray<TData, 3> triangle_normal(
    const FixedArray<FixedArray<TData, 3>, 3>& t,
    TriangleNormalErrorBehavior error_behavior = TriangleNormalErrorBehavior::RAISE)
{
    FixedArray<TData, 3> res = scaled_triangle_normal(t);
    TData ma = max(abs(res));
    if (ma < 1e-12) {
        if (error_behavior == TriangleNormalErrorBehavior::ZERO) {
            return fixed_zeros<TData, 3>();
        } else if (error_behavior == TriangleNormalErrorBehavior::WARN) {
            lwarn() << "Cannot calculate triangle normal";
            return fixed_zeros<TData, 3>();
        } else {
            throw TriangleException(t(0), t(1), t(2), "Cannot calculate triangle normal");
        }
    }
    res /= ma;
    res /= std::sqrt(sum(squared(res)));
    return res;
}

}
