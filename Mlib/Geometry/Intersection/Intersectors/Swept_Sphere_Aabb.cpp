#include "Swept_Sphere_Aabb.hpp"
#include <Mlib/Geometry/Intersection/Aabb_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Distange_Polygon_Aabb.hpp>
#include <Mlib/Math/Lerp.hpp>

using namespace Mlib;

template <class TData>
SweptSphereAabb<TData>::SweptSphereAabb(
    const FixedArray<TData, 3>& min,
    const FixedArray<TData, 3>& max,
    const TData& radius)
    : aabb_small_{ AxisAlignedBoundingBox<TData, 3>::from_min_max(min + radius, max - radius) }
    , aabb_large_{ AxisAlignedBoundingBox<TData, 3>::from_min_max(min, max) }
    , radius_{ radius }
    , bounding_sphere_{ FixedArray<TData, 2, 3>{ min, max } }
{
    if (any(aabb_small_.max() - aabb_small_.min() < (TData)0)) {
        THROW_OR_ABORT("SweptSphereAabb: AABB too small for the given radius");
    }
}

template <class TData>
BoundingSphere<TData, 3> SweptSphereAabb<TData>::bounding_sphere() const {
    return bounding_sphere_;
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const CollisionPolygonSphere<TData, 4>& q,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    if (!aabb_intersects_sphere(aabb_large_, q.bounding_sphere)) {
        return false;
    }
    ClosestPoint<TData> closest_point;
    distance_polygon_aabb(q, aabb_small_, closest_point);
    if (closest_point.distance <= radius_) {
        intersection_point = closest_point.closest_point0;
        normal = closest_point.normal;
        overlap = radius_ - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const CollisionPolygonSphere<TData, 3>& t,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    if (!aabb_intersects_sphere(aabb_large_, t.bounding_sphere)) {
        return false;
    }
    ClosestPoint<TData> closest_point;
    distance_polygon_aabb(t, aabb_small_, closest_point);
    if (closest_point.distance <= radius_) {
        intersection_point = closest_point.closest_point0;
        normal = closest_point.normal;
        overlap = radius_ - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const CollisionRidgeSphere<TData>& r1,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    if (!aabb_intersects_sphere(aabb_large_, r1.bounding_sphere)) {
        return false;
    }
    ClosestPoint<TData> closest_point;
    distance_line_aabb(r1.ray, aabb_small_, closest_point);
    if (closest_point.distance <= radius_) {
        intersection_point = closest_point.closest_point0;
        normal = closest_point.normal;
        overlap = radius_ - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const CollisionLineSphere<TData>& l1,
    TData& overlap,
    TData& ray_t,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    if (!aabb_intersects_sphere(aabb_large_, l1.bounding_sphere)) {
        return false;
    }
    ray_t = NAN;
    ClosestPoint<TData> closest_point;
    distance_line_aabb(l1.ray, aabb_small_, closest_point);
    if (closest_point.distance <= radius_) {
        intersection_point = closest_point.closest_point0;
        normal = closest_point.normal;
        overlap = radius_ - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const IIntersectable<TData>& intersectable,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    THROW_OR_ABORT("Sphere swept AABB called without transformation");
}

template <class TData>
bool SweptSphereAabb<TData>::intersects(
    const IIntersectable<TData>& intersectable,
    const TransformationMatrix<float, TData, 3>& trafo,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    const auto* c = dynamic_cast<const SweptSphereAabb<TData>*>(&intersectable);
    if (c == nullptr) {
        THROW_OR_ABORT("SweptSphereAabb can only intersect objects of type SweptSphereAabb");
    }
    ClosestPoint<TData> closest_point;
    distance_aabb_aabb(aabb_small_, c->aabb_small_, trafo, closest_point);
    auto sum_radius = radius_ + c->radius_;
    if (closest_point.distance <= sum_radius) {
        intersection_point = lerp(closest_point.closest_point0, closest_point.closest_point1, radius_ / sum_radius);
        normal = closest_point.normal;
        overlap = closest_point.distance - sum_radius;
        return true;
    } else {
        return false;
    }
}

namespace Mlib {

template class SweptSphereAabb<float>;

}