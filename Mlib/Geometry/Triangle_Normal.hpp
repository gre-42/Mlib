#pragma once
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <optional>

namespace Mlib {

template <class TData>
FixedArray<TData, 3> scaled_triangle_normal(const FixedArray<TData, 3, 3>& t)
{
    return cross(t[2] - t[1], t[0] - t[1]);
}

template <class TData>
std::optional<FixedArray<TData, 3>> try_triangle_normal(
    const FixedArray<TData, 3, 3>& t)
{
    FixedArray<TData, 3> res = scaled_triangle_normal(t);
    TData ma = max(abs(res));
    if (ma < 1e-12) {
        return std::nullopt;
    }
    res /= ma;
    res /= std::sqrt(sum(squared(res)));
    return res;
}

template <class TData>
FixedArray<TData, 3> triangle_normal(
    const FixedArray<TData, 3, 3>& t,
    NormalVectorErrorBehavior error_behavior = NormalVectorErrorBehavior::THROW)
{
    auto res = try_triangle_normal(t);
    if (!res.has_value()) {
        return get_alternative_or_throw<TData>(t, error_behavior);
    }
    return *res;
}

template <class TNormal, class TTriangle>
FixedArray<TNormal, 3> get_alternative_or_throw(
    const FixedArray<TTriangle, 3, 3>& t,
    NormalVectorErrorBehavior error_behavior) {
    if (any(error_behavior & NormalVectorErrorBehavior::WARN)) {
        lwarn() << "Cannot calculate triangle normal";
    }
    if (any(error_behavior & NormalVectorErrorBehavior::THROW)) {
        THROW_OR_ABORT2(TriangleException(t[0], t[1], t[2], "Cannot calculate triangle normal"));
    }
    return fixed_zeros<TNormal, 3>();
}

}
