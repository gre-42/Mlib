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

template <class TData>
class ClosestPoint {
public:
    ClosestPoint()
        : distance{ INFINITY }
    {}
    void update(
        const FixedArray<TData, 3>& candidate0,
        const FixedArray<TData, 3>& candidate1)
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
            normal = dir / distance;
        }
    }
    FixedArray<TData, 3> closest_point0 = uninitialized;
    FixedArray<TData, 3> closest_point1 = uninitialized;
    FixedArray<TData, 3> normal = uninitialized;
    TData distance;
};

template <class TData>
void distance_point_aabb(
    const FixedArray<TData, 3>& point,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    ClosestPoint<TData>& closest_point)
{
    // Point-volume
    closest_point.update(point, aabb.closest_point(point));
}

template <class TData>
void distance_interior_line_aabb(
    const RaySegment3D<TData>& ray,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    ClosestPoint<TData>& closest_point)
{
    // Line-point
    aabb.for_each_corner([&](const FixedArray<TData, 3>& corner){
        FixedArray<TData, 3> cp = uninitialized;
        TData ray_t;
        closest_point_to_line(corner, ray, ray_t, cp);
        if ((ray_t >= 0) && (ray_t <= ray.length)) {
            closest_point.update(cp, corner);
        }
        return true;
    });

    // Line-line
    auto line = FixedArray<TData, 2, 3>{ ray.start, ray.stop() };
    aabb.for_each_edge([&](const auto& e1){
        FixedArray<TData, 3> p0 = uninitialized;
        FixedArray<TData, 3> p1 = uninitialized;
        TData dist;
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

template <class TData>
void distance_line_aabb(
    const RaySegment3D<TData>& ray,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    ClosestPoint<TData>& closest_point)
{
    distance_point_aabb(ray.start, aabb, closest_point);
    distance_point_aabb(ray.stop(), aabb, closest_point);
    distance_interior_line_aabb(ray, aabb, closest_point);
}

template <class TData, size_t tnvertices>
void distance_interior_polygon_aabb(
    const ConvexPolygon3D<TData, tnvertices>& polygon,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    ClosestPoint<TData>& closest_point)
{
    // Plane-point
    aabb.for_each_corner([&](const FixedArray<TData, 3>& corner){
        if (polygon.contains(corner)) {
            auto proj = corner;
            const auto& n = polygon.plane().normal;
            proj -= n * (dot0d(n, corner) + polygon.plane().intercept);
            closest_point.update(proj, corner);
        }
        return true;
    });
}

template <class TData, size_t tnvertices>
void distance_polygon_aabb(
    const CollisionPolygonSphere<TData, tnvertices>& polygon,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    ClosestPoint<TData>& closest_point)
{
    // Point
    for (const auto& p : polygon.corners.row_iterable()) {
        distance_point_aabb(p, aabb, closest_point);
    }
    // Line
    for (size_t i = 0; i < tnvertices; ++i) {
        distance_interior_line_aabb(
            RaySegment3D<TData>{
                polygon.corners[i],
                polygon.corners[(i + 1) % tnvertices]
            },
            aabb,
            closest_point);
    }
    // Plane
    distance_interior_polygon_aabb(
        polygon.polygon,
        aabb,
        closest_point);
}

template <class TData>
void distance_aabb_aabb(
    const AxisAlignedBoundingBox<TData, 3>& aabb0,
    const AxisAlignedBoundingBox<TData, 3>& aabb1,
    const TransformationMatrix<float, TData, 3>& trafo1,
    ClosestPoint<TData>& closest_point)
{
    aabb1.for_each_corner([&](const FixedArray<TData, 3>& corner1){
        distance_point_aabb(trafo1.transform(corner1), aabb0, closest_point);
        return true;
    });
    aabb1.for_each_edge([&](const auto& e){
        auto te = trafo1.transform(e);
        distance_line_aabb({te[0], te[1]}, aabb0, closest_point);
        return true;
    });
    aabb1.for_each_face([&](const auto& p){
        distance_interior_polygon_aabb(p.transformed(trafo1), aabb0, closest_point);
        return true;
    });
}

}

#ifdef __GNUC__
#pragma GCC pop_options
#endif
