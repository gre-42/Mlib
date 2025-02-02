#include "Swept_Sphere_Aabb.hpp"
#include <Mlib/Geometry/Intersection/Aabb_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Distange_Polygon_Aabb.hpp>
#include <Mlib/Math/Lerp.hpp>

using namespace Mlib;

SweptSphereAabb::SweptSphereAabb(
    const FixedArray<CompressedScenePos, 3>& min,
    const FixedArray<CompressedScenePos, 3>& max,
    const CompressedScenePos& radius)
    : aabb_small_{ AxisAlignedBoundingBox<CompressedScenePos, 3>::from_min_max(min + radius, max - radius) }
    , aabb_large_{ AxisAlignedBoundingBox<CompressedScenePos, 3>::from_min_max(min, max) }
    , radius_{ radius }
    , bounding_sphere_{ FixedArray<CompressedScenePos, 2, 3>{ min, max } }
{
    if (any(aabb_small_.size() < (CompressedScenePos)1e-3f)) {
        THROW_OR_ABORT("SweptSphereAabb: AABB too small for the given radius");
    }
}

BoundingSphere<CompressedScenePos, 3> SweptSphereAabb::bounding_sphere() const {
    return bounding_sphere_;
}

AxisAlignedBoundingBox<CompressedScenePos, 3> SweptSphereAabb::aabb() const {
    return aabb_large_;
}

bool SweptSphereAabb::intersects(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    if (!aabb_large_.intersects(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(q.corners))) {
        return false;
    }
    ClosestPoint<SceneDir, ScenePos> closest_point;
    distance_polygon_aabb(q, aabb_small_.casted<ScenePos>(), closest_point);
    if (closest_point.distance <= (ScenePos)radius_) {
        intersection_point = closest_point.closest_point0.casted<ScenePos>();
        normal = closest_point.normal;
        overlap = ScenePos(radius_) - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

bool SweptSphereAabb::intersects(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    if (!aabb_large_.intersects(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(t.corners))) {
        return false;
    }
    ClosestPoint<SceneDir, ScenePos> closest_point;
    distance_polygon_aabb(t, aabb_small_.casted<ScenePos>(), closest_point);
    if (closest_point.distance <= (ScenePos)radius_) {
        intersection_point = closest_point.closest_point0.casted<ScenePos>();
        normal = closest_point.normal;
        overlap = ScenePos(radius_) - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

bool SweptSphereAabb::intersects(
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    if (!aabb_large_.intersects(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(r1.edge))) {
        return false;
    }
    ClosestPoint<SceneDir, ScenePos> closest_point;
    distance_line_aabb(r1.ray.casted<SceneDir, ScenePos>(), aabb_small_.casted<ScenePos>(), closest_point);
    if (closest_point.distance <= (ScenePos)radius_) {
        intersection_point = closest_point.closest_point0.casted<ScenePos>();
        normal = closest_point.normal;
        overlap = ScenePos(radius_) - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

bool SweptSphereAabb::intersects(
    const CollisionLineSphere<CompressedScenePos>& l1,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    if (!aabb_large_.intersects(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(l1.line))) {
        return false;
    }
    ray_t = NAN;
    ClosestPoint<SceneDir, ScenePos> closest_point;
    distance_line_aabb(l1.ray.casted<SceneDir, ScenePos>(), aabb_small_.casted<ScenePos>(), closest_point);
    if (closest_point.distance <= (ScenePos)radius_) {
        intersection_point = closest_point.closest_point0.casted<ScenePos>();
        normal = closest_point.normal;
        overlap = ScenePos(radius_) - closest_point.distance;
        return true;
    } else {
        return false;
    }
}

bool SweptSphereAabb::intersects(
    const IIntersectable& intersectable,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    THROW_OR_ABORT("Sphere swept AABB called without transformation");
}

bool SweptSphereAabb::intersects(
    const IIntersectable& intersectable,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    const auto* c = dynamic_cast<const SweptSphereAabb*>(&intersectable);
    if (c == nullptr) {
        THROW_OR_ABORT("SweptSphereAabb can only intersect objects of type SweptSphereAabb");
    }
    ClosestPoint<SceneDir, ScenePos> closest_point;
    distance_aabb_aabb(aabb_small_.casted<ScenePos>(), c->aabb_small_.casted<ScenePos>(), trafo, closest_point);
    auto sum_radius = radius_ + c->radius_;
    if (closest_point.distance <= (ScenePos)sum_radius) {
        intersection_point = lerp(
            closest_point.closest_point0.casted<ScenePos>(),
            closest_point.closest_point1.casted<ScenePos>(),
            (ScenePos)radius_ / (ScenePos)(sum_radius));
        normal = closest_point.normal;
        overlap = (ScenePos)sum_radius - closest_point.distance;
        return true;
    } else {
        return false;
    }
}
