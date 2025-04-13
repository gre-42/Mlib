#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Ray_Segment_3D_For_Aabb.hpp>

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <size_t tnvertices>
bool intersect_polygon_aabb(
    const CollisionPolygonSphere<ScenePos, tnvertices>& polygon,
    const AxisAlignedBoundingBox<ScenePos, 3>& aabb)
{
    // Polygon point
    for (const auto& p : polygon.corners.row_iterable()) {
        if (aabb.contains(p)) {
            return true;
        }
    }
    // Polygon line
    for (size_t i = 0; i < tnvertices; ++i) {
        auto ray = RaySegment3DForAabb(RaySegment3D<SceneDir, ScenePos>{
            RaySegment3D<SceneDir, ScenePos>{
                polygon.corners[i].template casted<ScenePos>(),
                polygon.corners[(i + 1) % tnvertices].template casted<ScenePos>()
            }});
        if (ray.intersects(aabb)) {
            return true;
        }
    }
    // AABB line
    auto poly = polygon.polygon.template casted<ScenePos, ScenePos>();
    if (!aabb.for_each_ray<ScenePos>([&](const RaySegment3D<ScenePos, ScenePos>& ray){
        ScenePos t;
        FixedArray<ScenePos, 3> intersection_point = uninitialized;
        return !ray.intersects(poly, &t, &intersection_point);
    }))
    {
        return true;
    }
    return false;
}

template <class TDir, class TPos>
bool intersect_aabb_aabb(
    const AxisAlignedBoundingBox<TPos, 3>& aabb0,
    const AxisAlignedBoundingBox<TPos, 3>& aabb1,
    const TransformationMatrix<TDir, TPos, 3>& trafo1)
{
    auto itrafo1 = trafo1.inverted();
    if (!aabb1.for_each_corner([&](const FixedArray<TPos, 3>& corner1){
        return !aabb0.contains(trafo1.transform(corner1));
    }))
    {
        return true;
    }
    if (!aabb0.for_each_corner([&](const FixedArray<TPos, 3>& corner0){
        return !aabb1.contains(itrafo1.transform(corner0));
    }))
    {
        return true;
    }
    if (!aabb1.template for_each_ray<TDir>([&](const auto& ray1){
        auto transformed_ray1 = RaySegment3DForAabb{ray1.transformed(trafo1)};
        return !transformed_ray1.intersects(aabb0);
    }))
    {
        return true;
    }
    if (!aabb0.template for_each_ray<TDir>([&](const auto& ray0){
        auto transformed_ray0 = RaySegment3DForAabb{ray0.transformed(itrafo1)};
        return !transformed_ray0.intersects(aabb1);
    }))
    {
        return true;
    }
    return false;
}

}

#ifdef __GNUC__
#pragma GCC pop_options
#endif
