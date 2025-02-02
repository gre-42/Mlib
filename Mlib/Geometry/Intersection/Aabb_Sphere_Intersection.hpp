#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

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

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
