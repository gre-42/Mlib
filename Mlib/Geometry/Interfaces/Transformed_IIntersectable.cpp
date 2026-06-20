#include "Transformed_IIntersectable.hpp"
#include <Mlib/Geometry/Mesh/Save_Polygon_To_Obj.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Primitives/Collision_Line.hpp>
#include <Mlib/Geometry/Primitives/Collision_Polygon.hpp>
#include <Mlib/Geometry/Primitives/Collision_Ridge.hpp>
#include <Mlib/Geometry/Primitives/Intersectors/Aabb_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Primitives/Intersectors/Closest_Point_On_Intersection.hpp>
#include <Mlib/Geometry/Primitives/Intersectors/Intersection_Status.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>
#include <Mlib/Os/Env.hpp>

PRAGMA_GCC_O3_BEGIN

using namespace Mlib;

TransformedIntersectable::TransformedIntersectable(
    std::shared_ptr<IIntersectable> child,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo)
    : child_{ std::move(child) }
    , trafo_{ trafo }
{}

BoundingSphere<CompressedScenePos, 3> TransformedIntersectable::bounding_sphere() const {
    return child_->bounding_sphere().transformed(trafo_);
}

AxisAlignedBoundingBox<CompressedScenePos, 3> TransformedIntersectable::aabb() const {
    return child_->aabb().template casted<ScenePos>().transformed(trafo_).casted<CompressedScenePos>();
}

std::shared_ptr<IIntersectable> TransformedIntersectable::sweep(
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    throw std::runtime_error("TransformedIntersectable cannot sweep");
}

IntersectionStatus TransformedIntersectable::touches(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    return touches_any_wo_ray_t(q, overlap, intersection_point, normal, on_intersection);
}

IntersectionStatus TransformedIntersectable::touches(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    return touches_any_wo_ray_t(t, overlap, intersection_point, normal, on_intersection);
}

IntersectionStatus TransformedIntersectable::touches(
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    return touches_any_wo_ray_t(r1, overlap, intersection_point, normal, on_intersection);
}

IntersectionStatus TransformedIntersectable::touches(
    const CollisionLineSphere<CompressedScenePos>& l1,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    return touches_any_with_ray_t(l1, overlap, ray_t, intersection_point, normal, on_intersection);
}

IntersectionStatus TransformedIntersectable::touches(
    const IIntersectable& intersectable,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    auto* o = dynamic_cast<const TransformedIntersectable*>(&intersectable);
    if (o == nullptr) {
        throw std::runtime_error("TransformedIntersectable can only intersect children of the same type (0)");
    }
    ScenePos c_overlap;
    FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
    FixedArray<SceneDir, 3> c_normal = uninitialized;
    auto status = child_->touches(
        *o->child_,
        (trafo_.inverted() * o->trafo_).template casted<SceneDir, ScenePos>(),
        c_overlap,
        c_intersection_point,
        c_normal,
        on_intersection);
    switch (status) {
    case IntersectionStatus::SEPARATE:
    case IntersectionStatus::OVERLAP_TOO_LARGE:
        return status;
    case IntersectionStatus::COLLISION:
        overlap = (ScenePos)c_overlap;
        intersection_point = trafo_.transform(c_intersection_point);
        normal = trafo_.rotate(c_normal.template casted<SceneDir>());
        return status;
    }
    throw std::runtime_error("Unknown intersection status");
}

IntersectionStatus TransformedIntersectable::touches(
    const IIntersectable& intersectable,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    throw std::runtime_error("TransformedIIntersectable received additional transformation matrix (0)");
}

bool TransformedIntersectable::can_spawn_at(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t) const
{
    return can_spawn_at_any(t);
}

bool TransformedIntersectable::can_spawn_at(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q) const
{
    return can_spawn_at_any(q);
}

bool TransformedIntersectable::can_spawn_at(
    const IIntersectable& intersectable) const
{
    auto* o = dynamic_cast<const TransformedIntersectable*>(&intersectable);
    if (o == nullptr) {
        throw std::runtime_error("TransformedIntersectable can only intersect children of the same type (1)");
    }
    return child_->can_spawn_at(
        *o->child_,
        (trafo_.inverted() * o->trafo_).template casted<SceneDir, ScenePos>());
}

bool TransformedIntersectable::can_spawn_at(
    const IIntersectable& intersectable,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const
{
    throw std::runtime_error("TransformedIIntersectable received additional transformation matrix (1)");
}

template <class TOther>
IntersectionStatus TransformedIntersectable::touches_any_wo_ray_t(
    const TOther& o,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    try {
        auto tbs = o.bounding_sphere.itransformed(trafo_);
        if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
            return IntersectionStatus::SEPARATE;
        }
        ScenePos c_overlap;
        FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
        FixedArray<SceneDir, 3> c_normal = uninitialized;
        auto status = child_->touches(
            o.transformed(trafo_.inverted()),
            c_overlap,
            c_intersection_point,
            c_normal,
            on_intersection);
        switch (status) {
        case IntersectionStatus::SEPARATE:
        case IntersectionStatus::OVERLAP_TOO_LARGE:
            return status;
        case IntersectionStatus::COLLISION:
            overlap = (ScenePos)c_overlap;
            intersection_point = trafo_.transform(c_intersection_point);
            normal = trafo_.rotate(c_normal.template casted<SceneDir>());
            return status;
        }
        throw std::runtime_error("Unknown intersection status");
    } catch (const std::runtime_error& e) {
        if (auto e = try_getenv("POLYGON_FILENAME"); e.has_value()) {
            save_polygon_to_obj(*e, o.transformed(trafo_.inverted()).corners.template casted<double>());
        }
        throw std::runtime_error((std::stringstream() <<
            "touches_any_wo_ray_t: position [m]: " << (trafo_.t / meters) <<
            ", rotation [°]: " << (matrix_2_tait_bryan_angles(trafo_.R) / degrees) <<
            ": " << e.what() <<
            "\nConsider setting the POLYGON_FILENAME environment variable").str());
    }
}

template <class TOther>
IntersectionStatus TransformedIntersectable::touches_any_with_ray_t(
    const TOther& o,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal,
    ClosestPointOnIntersection on_intersection) const
{
    try {
        auto tbs = o.bounding_sphere.itransformed(trafo_);
        if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
            return IntersectionStatus::SEPARATE;
        }
        ScenePos c_overlap;
        ScenePos c_ray_t;
        FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
        FixedArray<SceneDir, 3> c_normal = uninitialized;
        auto status = child_->touches(
            o.transformed(trafo_.inverted()),
            c_overlap,
            c_ray_t,
            c_intersection_point,
            c_normal,
            on_intersection);
        switch (status) {
        case IntersectionStatus::SEPARATE:
        case IntersectionStatus::OVERLAP_TOO_LARGE:
            return status;
        case IntersectionStatus::COLLISION:
            overlap = (ScenePos)c_overlap;
            ray_t = (ScenePos)c_ray_t;
            intersection_point = trafo_.transform(c_intersection_point);
            normal = trafo_.rotate(c_normal.template casted<SceneDir>());
            return status;
        }
        throw std::runtime_error("Unknown intersection status");
    } catch (const std::runtime_error& e) {
        if (auto e = try_getenv("POLYGON_FILENAME"); e.has_value()) {
            save_polygon_to_obj(*e, o.transformed(trafo_.inverted()).corners.template casted<double>());
        }
        throw std::runtime_error((std::stringstream() <<
            "touches_any_with_ray_t: position [m]: " << (trafo_.t / meters) <<
            ", rotation [°]: " << (matrix_2_tait_bryan_angles(trafo_.R) / degrees) <<
            ": " << e.what() <<
            "\nConsider setting the POLYGON_FILENAME environment variable").str());
    }
}

template <class TOther>
bool TransformedIntersectable::can_spawn_at_any(const TOther& o) const {
    auto tbs = o.bounding_sphere.itransformed(trafo_);
    if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
        return false;
    }
    return child_->can_spawn_at(o.transformed(trafo_.inverted()));
}

PRAGMA_GCC_O3_END
