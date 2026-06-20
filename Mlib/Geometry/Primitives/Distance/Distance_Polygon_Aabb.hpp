#pragma once
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Collision_Polygon.hpp>
#include <Mlib/Geometry/Primitives/Convex_Polygon.hpp>
#include <Mlib/Geometry/Primitives/Distance/Distance_Line_Line.hpp>
#include <Mlib/Geometry/Primitives/Distance/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Primitives/Intersectors/Closest_Point_On_Intersection.hpp>
#include <Mlib/Geometry/Primitives/Intersectors/Intersection_Status.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>

PRAGMA_GCC_O3_BEGIN

namespace Mlib {

enum class ClosestPointStatus {
    SUCCESS,
    INTERSECT
};

template <class TDir, class TPos>
class ClosestPoint {
public:
    explicit ClosestPoint(ClosestPointOnIntersection on_intersection)
        : distance{ INFINITY }
        , on_intersection_{ on_intersection }
    {}
    [[ nodiscard ]] ClosestPointStatus update(
        const FixedArray<TPos, 3>& candidate0,
        const FixedArray<TPos, 3>& candidate1)
    {
        auto dir = candidate0 - candidate1;
        auto dist2 = sum(squared(dir));
        if (dist2 < 1e-12) {
            if (on_intersection_ == ClosestPointOnIntersection::THROW) {
                throw std::runtime_error("Polygon intersects AABB");
            }
            return ClosestPointStatus::INTERSECT;
        }
        if (dist2 < squared(distance)) {
            closest_point0 = candidate0;
            closest_point1 = candidate1;
            distance = std::sqrt(dist2);
            normal = (dir / distance).template casted<TDir>();
        }
        return ClosestPointStatus::SUCCESS;
    }
    FixedArray<TPos, 3> closest_point0 = uninitialized;
    FixedArray<TPos, 3> closest_point1 = uninitialized;
    FixedArray<TDir, 3> normal = uninitialized;
    TPos distance;
private:
    ClosestPointOnIntersection on_intersection_;
};

template <class TDir, class TPos>
[[ nodiscard ]] ClosestPointStatus distance_point_aabb(
    const FixedArray<TPos, 3>& point,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Point-volume
    try {
        return closest_point.update(point, aabb.closest_point(point));
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("distance_point_aabb, point-volume failed: " + std::string(e.what()));
    }
}

template <class TDir, class TPos>
[[ nodiscard ]] ClosestPointStatus distance_interior_line_aabb(
    const RaySegment3D<TDir, TPos>& ray,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Line-point
    if (!aabb.for_each_corner([&](const FixedArray<TPos, 3>& corner){
        FixedArray<TPos, 3> cp = uninitialized;
        TPos ray_t;
        closest_point_to_line(corner, ray, ray_t, cp);
        if ((ray_t >= 0) && (ray_t <= ray.length)) {
            try {
                return (closest_point.update(cp, corner) == ClosestPointStatus::SUCCESS);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("distance_interior_line_aabb, line-point failed: " + std::string(e.what()));
            }
        }
        return true;
    }))
    {
        return ClosestPointStatus::INTERSECT;
    }

    // Line-line
    auto line = FixedArray<TPos, 2, 3>{ ray.start, ray.stop() };
    if (!aabb.for_each_edge([&](const auto& e1){
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
            try {
                return (closest_point.update(p0, p1) == ClosestPointStatus::SUCCESS);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("distance_interior_line_aabb, line-line failed: " + std::string(e.what()));
            }
        }
        return true;
    }))
    {
        return ClosestPointStatus::INTERSECT;
    }
    return ClosestPointStatus::SUCCESS;
}

template <class TDir, class TPos>
[[ nodiscard ]] ClosestPointStatus distance_line_aabb(
    const RaySegment3D<TDir, TPos>& ray,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    try 
    {
        if (distance_point_aabb(ray.start, aabb, closest_point) != ClosestPointStatus::SUCCESS) {
            return ClosestPointStatus::INTERSECT;
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("distance_line_aabb, distance_point_aabb(ray.start, ...) failed: " + std::string(e.what()));
    }
    try {
        if (distance_point_aabb(ray.stop(), aabb, closest_point) != ClosestPointStatus::SUCCESS) {
            return ClosestPointStatus::INTERSECT;
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("distance_line_aabb, distance_point_aabb(ray.stop, ...) failed: " + std::string(e.what()));
    }
    try {
        if (distance_interior_line_aabb(ray, aabb, closest_point) != ClosestPointStatus::SUCCESS) {
            return ClosestPointStatus::INTERSECT;
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("distance_line_aabb, distance_interior_line_aabb failed: " + std::string(e.what()));
    }
    return ClosestPointStatus::SUCCESS;
}

template <class TDir, class TPos, size_t tnvertices>
[[ nodiscard ]] ClosestPointStatus distance_interior_polygon_aabb(
    const ConvexPolygon3D<TDir, TPos, tnvertices>& polygon,
    const AxisAlignedBoundingBox<TPos, 3>& aabb,
    ClosestPoint<TDir, TPos>& closest_point)
{
    // Plane-point
    if (!aabb.for_each_corner([&](const FixedArray<TPos, 3>& corner){
        if (polygon.contains(corner)) {
            using I = funpack_t<TPos>;
            auto proj = corner.template casted<I>();
            auto n = polygon.plane.normal.template casted<I>();
            proj -= n * (dot0d(n, proj) + (I)polygon.plane.intercept);
            try {
                return (closest_point.update(proj.template casted<TPos>(), corner) == ClosestPointStatus::SUCCESS);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("distance_interior_polygon_aabb, plane-point failed: " + std::string(e.what()));
            }
        }
        return true;
    }))
    {
        return ClosestPointStatus::INTERSECT;
    }
    return ClosestPointStatus::SUCCESS;
}

template <size_t tnvertices>
[[ nodiscard ]] ClosestPointStatus distance_polygon_aabb(
    const CollisionPolygonSphere<CompressedScenePos, tnvertices>& polygon,
    const AxisAlignedBoundingBox<ScenePos, 3>& aabb,
    ClosestPoint<SceneDir, ScenePos>& closest_point)
{
    try {
        // Point
        for (const auto& p : polygon.corners.row_iterable()) {
            auto status = distance_point_aabb(p.template casted<ScenePos>(), aabb, closest_point);
            if (status == ClosestPointStatus::INTERSECT) {
                return ClosestPointStatus::INTERSECT;
            }
        }
        // Line
        for (size_t i = 0; i < tnvertices; ++i) {
            auto status = distance_interior_line_aabb(
                RaySegment3D<SceneDir, ScenePos>{
                    polygon.corners[i].template casted<ScenePos>(),
                    polygon.corners[(i + 1) % tnvertices].template casted<ScenePos>()
                },
                aabb.template casted<ScenePos>(),
                closest_point);
            if (status == ClosestPointStatus::INTERSECT) {
                return ClosestPointStatus::INTERSECT;
            }
        }
        // Plane
        {
            auto status = distance_interior_polygon_aabb(
                polygon.polygon.template casted<SceneDir, ScenePos>(),
                aabb.template casted<ScenePos>(),
                closest_point);
            if (status == ClosestPointStatus::INTERSECT) {
                return ClosestPointStatus::INTERSECT;
            }
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            (std::stringstream() << "distance_polygon_aabb failed. Polygon: " <<
            polygon.corners << ", AABB: " << aabb << ", message: " << e.what()).str());
    }
    return ClosestPointStatus::SUCCESS;
}

template <class TDir, class TPos>
[[ nodiscard ]] ClosestPointStatus distance_aabb_aabb(
    const AxisAlignedBoundingBox<TPos, 3>& aabb0,
    const AxisAlignedBoundingBox<TPos, 3>& aabb1,
    const TransformationMatrix<TDir, TPos, 3>& trafo1,
    ClosestPoint<TDir, TPos>& closest_point)
{
    try {
        if (!aabb1.for_each_corner([&](const FixedArray<TPos, 3>& corner1){
            return (distance_point_aabb(trafo1.transform(corner1), aabb0, closest_point) == ClosestPointStatus::SUCCESS);
        }))
        {
            return ClosestPointStatus::INTERSECT;
        }
        if (!aabb1.for_each_edge([&](const auto& e){
            auto te = trafo1.transform(e);
            return (distance_line_aabb({te[0], te[1]}, aabb0, closest_point) == ClosestPointStatus::SUCCESS);
        }))
        {
            return ClosestPointStatus::INTERSECT;
        }
        if (!aabb1.template for_each_face<float>([&](const auto& p){
            return distance_interior_polygon_aabb(
                p.transformed(trafo1),
                aabb0,
                closest_point) == ClosestPointStatus::SUCCESS;
        }))
        {
            return ClosestPointStatus::INTERSECT;
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            (std::stringstream() << "distance_aabb_aabb failed. Transformation1: " <<
            trafo1 << ", AABB0: " << aabb0 << ", AABB1: " << aabb1 << ", message: " << e.what()).str());
    }
    return ClosestPointStatus::SUCCESS;
}

}

PRAGMA_GCC_O3_END
