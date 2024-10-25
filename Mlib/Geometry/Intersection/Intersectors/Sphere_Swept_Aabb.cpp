#include "Sphere_Swept_Aabb.hpp"
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Distange_Polygon_Aabb.hpp>

using namespace Mlib;

template <class TData>
SphereSweptAabb<TData>::SphereSweptAabb(
    const FixedArray<TData, 3>& min,
    const FixedArray<TData, 3>& max,
    const TData& radius)
    : aabb_{ AxisAlignedBoundingBox<TData, 3>::from_min_max(min, max) }
    , radius_{ radius }
    , bounding_sphere_{ FixedArray<TData, 2, 3>{ min, max } }
{}

template <class TData>
BoundingSphere<TData, 3> SphereSweptAabb<TData>::bounding_sphere() const {
    return bounding_sphere_;
}

template <class TData>
bool SphereSweptAabb<TData>::intersects(
    const CollisionPolygonSphere<TData, 4>& q,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    ClosestPoint<TData> closest_point;
    distance_polygon_aabb(q.polygon, aabb_, intersection_point, normal, overlap);
    if (closest_point.overlap != INFINITY) {
        intersection_point = closest_point.intersection_point;
        normal = closest_point.normal;
        overlap = closest_point.overlap;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SphereSweptAabb<TData>::intersects(
    const CollisionPolygonSphere<TData, 3>& t,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    ClosestPoint<TData> closest_point;
    distance_polygon_aabb(t.polygon, aabb_, intersection_point, normal, overlap);
    if (closest_point.overlap != INFINITY) {
        intersection_point = closest_point.intersection_point;
        normal = closest_point.normal;
        overlap = closest_point.overlap;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SphereSweptAabb<TData>::intersects(
    const CollisionRidgeSphere<TData>& r1,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    ClosestPoint<TData> closest_point;
    distance_line_aabb(r1.line, aabb_, intersection_point, normal, overlap);
    if (closest_point.overlap != INFINITY) {
        intersection_point = closest_point.intersection_point;
        normal = closest_point.normal;
        overlap = closest_point.overlap;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SphereSweptAabb<TData>::intersects(
    const CollisionLineSphere<TData>& l1,
    TData& overlap,
    TData& ray_t,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    ray_t = NAN;
    ClosestPoint<TData> closest_point;
    distance_line_aabb(l1.line, aabb_, closest_point);
    if (closest_point.overlap != INFINITY) {
        intersection_point = closest_point.intersection_point;
        normal = closest_point.normal;
        overlap = closest_point.overlap;
        return true;
    } else {
        return false;
    }
}

template <class TData>
bool SphereSweptAabb<TData>::intersects(
    const IIntersectable<TData>& intersectable,
    const TransformationMatrix<float, TData, 3>& trafo,
    TData& overlap,
    FixedArray<TData, 3>& intersection_point,
    FixedArray<TData, 3>& normal) const
{
    auto* c = dynamic_cast<SphereSweptAabb<TData>*>(&intersectable);
    if (c == nullptr) {
        THROW_OR_ABORT("SphereSweptAabb can only intersect objects of type SphereSweptAabb");
    }
    ClosestPoint<TData> closest_point;
    distance_aabb_aabb(aabb_, c->aabb, trafo, overlap, closest_point);
    if (closest_point.overlap != INFINITY) {
        intersection_point = closest_point.intersection_point;
        normal = closest_point.normal;
        overlap = closest_point.overlap;
        return true;
    } else {
        return false;
    }
}
