#pragma once
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>

PRAGMA_GCC_O3_BEGIN

namespace Mlib {

template <class TData, size_t tndim>
bool aabb_intersects_sphere(
    const AxisAlignedBoundingBox<TData, tndim>& aabb,
    const BoundingSphere<TData, tndim>& sphere)
{
    auto distance1 = abs(aabb.closest_point(sphere.center) - sphere.center);
    if (any(distance1 > sphere.radius)) {
        return false;
    }
    auto distance2 = sum(squared(funpack(distance1)));
    return distance2 <= squared(funpack(sphere.radius));
}

}

PRAGMA_GCC_O3_END
