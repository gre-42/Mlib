#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Distance_Line_Line.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TDir, class TPos>
class ClosestPoint {
public:
    ClosestPoint()
        : distance{ INFINITY }
    {}
    void update(
        const FixedArray<TPos, 3>& candidate0,
        const FixedArray<TPos, 3>& candidate1)
    {
        auto dir = candidate0 - candidate1;
        auto dist2 = sum(squared(dir));
        if (dist2 < 1e-12) {
            THROW_OR_ABORT("Polygon intersects AABB");
        }
        if (dist2 < squared(distance)) {
            closest_point0 = candidate0;
            closest_point1 = candidate1;
            distance = std::sqrt(dist2);
            normal = (dir / distance).template casted<TDir>();
        }
    }
    FixedArray<TPos, 3> closest_point0 = uninitialized;
    FixedArray<TPos, 3> closest_point1 = uninitialized;
    FixedArray<TDir, 3> normal = uninitialized;
    TPos distance;
};

template <class TDir, class TPos>
void distance_point_aabb(
    const FixedArray<TPos, 3>& point,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Point-volume
    closest_point.update(point, aabb.closest_point(point));
}

template <class TDir, class TPos>
void distance_interior_line_aabb(
    const RaySegment3D<TDir, TPos>& ray,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Line-point
    aabb.for_each_corner([&](const FixedArray<TPos, 3>& corner){
        FixedArray<TPos, 3> cp = uninitialized;
        TPos ray_t;
        closest_point_to_line(corner, ray, ray_t, cp);
        if ((ray_t >= 0) && (ray_t <= ray.length)) {
            closest_point.update(cp, corner);
        }
        return true;
    });

    // Line-line
    auto line = FixedArray<TPos, 2, 3>{ ray.start, ray.stop() };
    aabb.for_each_edge([&](const auto& e1){
        FixedArray<TPos, 3> p0 = uninitialized;
        FixedArray<TPos, 3> p1 = uninitialized;
        TPos dist;
        if (!distance_line_line(line, e1, dist)) {
            return true;
        }
        if (dist >= closest_point.distance) {
            return true;
        }
        if (distance_line_line(line, e1, p0, p1)) {
            closest_point.update(p0, p1);
        }
        return true;
    });
}

template <class TDir, class TPos>
void distance_line_aabb(
    const RaySegment3D<TDir, TPos>& ray,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    distance_point_aabb(ray.start, aabb, closest_point);
    distance_point_aabb(ray.stop(), aabb, closest_point);
    distance_interior_line_aabb(ray, aabb, closest_point);
}

template <class TDir, class TPos, size_t tnvertices>
void distance_interior_polygon_aabb(
    const ConvexPolygon3D<TDir, TPos, tnvertices>& polygon,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Plane-point
    aabb.for_each_corner([&](const FixedArray<TPos, 3>& corner){
        if (polygon.contains(corner)) {
            using I = funpack_t<TPos>;
            auto proj = corner.template casted<I>();
            auto n = polygon.plane.normal.template casted<I>();
            proj -= n * (dot0d(n, proj) + (I)polygon.plane.intercept);
            closest_point.update(proj.template casted<TPos>(), corner);
        }
        return true;
    });
}

template <size_t tnvertices>
void distance_polygon_aabb(
    const CollisionPolygonSphere<CompressedScenePos, tnvertices>& polygon,
    const AxisAlignedBoundingBox<ScenePos, 3>& aabb,
    ClosestPoint<SceneDir, ScenePos>& closest_point)
{
    // Point
    for (const auto& p : polygon.corners.row_iterable()) {
        distance_point_aabb(p.template casted<ScenePos>(), aabb, closest_point);
    }
    // Line
    for (size_t i = 0; i < tnvertices; ++i) {
        distance_interior_line_aabb(
            RaySegment3D<SceneDir, ScenePos>{
                polygon.corners[i].template casted<ScenePos>(),
                polygon.corners[(i + 1) % tnvertices].template casted<ScenePos>()
            },
            aabb.template casted<ScenePos>(),
            closest_point);
    }
    // Plane
    distance_interior_polygon_aabb(
        polygon.polygon.template casted<SceneDir, ScenePos>(),
        aabb.template casted<ScenePos>(),
        closest_point);
}

template <class TDir, class TPos>
void distance_aabb_aabb(
    const AxisAlignedBoundingBox<TPos, 3>& aabb0,
    const AxisAlignedBoundingBox<TPos, 3>& aabb1,
    const TransformationMatrix<TDir, TPos, 3>& trafo1,
    ClosestPoint<TDir, TPos>& closest_point)
{
    aabb1.for_each_corner([&](const FixedArray<TPos, 3>& corner1){
        distance_point_aabb(trafo1.transform(corner1), aabb0, closest_point);
        return true;
    });
    aabb1.for_each_edge([&](const auto& e){
        auto te = trafo1.transform(e);
        distance_line_aabb({te[0], te[1]}, aabb0, closest_point);
        return true;
    });
    aabb1.template for_each_face<float>([&](const auto& p){
        distance_interior_polygon_aabb(
            p.transformed(trafo1),
            aabb0,
            closest_point);
        return true;
    });
}

}

#ifdef __GNUC__
#pragma GCC pop_options
#endif
