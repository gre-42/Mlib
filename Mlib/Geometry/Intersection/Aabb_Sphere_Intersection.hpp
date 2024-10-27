#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

template <class TData, size_t tndim>
bool aabb_intersects_sphere(
	const AxisAlignedBoundingBox<TData, tndim>& aabb,
	const BoundingSphere<TData, tndim>& sphere)
{
	auto distance1 = abs(aabb.closest_point(sphere.center()) - sphere.center());
	if (any(distance1 > sphere.radius())) {
		return false;
	}
	auto distance2 = sum(squared(distance1));
	return distance2 <= squared(sphere.radius());
}

}
