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

}

#ifdef __GNUC__
#pragma GCC pop_options
#endif
