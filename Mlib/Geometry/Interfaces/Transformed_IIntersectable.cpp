#include "Transformed_IIntersectable.hpp"
#include <Mlib/Geometry/Intersection/Aabb_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

TransformedIntersectable::TransformedIntersectable(
    std::shared_ptr<IIntersectable> child,
    const TransformationMatrix<float, ScenePos, 3>& trafo)
    : child_{ child }
    , trafo_{ trafo }
{}

BoundingSphere<CompressedScenePos, 3> TransformedIntersectable::bounding_sphere() const {
    return child_->bounding_sphere().transformed(trafo_);
}

AxisAlignedBoundingBox<CompressedScenePos, 3> TransformedIntersectable::aabb() const {
    return child_->aabb().template casted<ScenePos>().transformed(trafo_).casted<CompressedScenePos>();
}

bool TransformedIntersectable::touches(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    return touches_any_wo_ray_t(q, overlap, intersection_point, normal);
}

bool TransformedIntersectable::touches(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    return touches_any_wo_ray_t(t, overlap, intersection_point, normal);
}

bool TransformedIntersectable::touches(
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    return touches_any_wo_ray_t(r1, overlap, intersection_point, normal);
}

bool TransformedIntersectable::touches(
    const CollisionLineSphere<CompressedScenePos>& l1,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    return touches_any_with_ray_t(l1, overlap, ray_t, intersection_point, normal);
}

bool TransformedIntersectable::touches(
    const IIntersectable& intersectable,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    auto* o = dynamic_cast<const TransformedIntersectable*>(&intersectable);
    if (o == nullptr) {
        THROW_OR_ABORT("TransformedIntersectable can only intersect children of the same type (0)");
    }
    ScenePos c_overlap;
    FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
    FixedArray<SceneDir, 3> c_normal = uninitialized;
    bool intersects = child_->touches(
        *o->child_,
        (trafo_.inverted() * o->trafo_).template casted<float, ScenePos>(),
        c_overlap,
        c_intersection_point,
        c_normal);
    if (!intersects) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    intersection_point = trafo_.transform(c_intersection_point);
    normal = trafo_.rotate(c_normal.template casted<float>());
    return true;
}

bool TransformedIntersectable::touches(
    const IIntersectable& intersectable,
    const TransformationMatrix<float, ScenePos, 3>& trafo,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    THROW_OR_ABORT("TransformedIIntersectable received additional transformation matrix (0)");
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
        THROW_OR_ABORT("TransformedIntersectable can only intersect children of the same type (1)");
    }
    return child_->can_spawn_at(
        *o->child_,
        (trafo_.inverted() * o->trafo_).template casted<float, ScenePos>());
}

bool TransformedIntersectable::can_spawn_at(
    const IIntersectable& intersectable,
    const TransformationMatrix<float, ScenePos, 3>& trafo) const
{
    THROW_OR_ABORT("TransformedIIntersectable received additional transformation matrix (1)");
}

template <class TOther>
bool TransformedIntersectable::touches_any_wo_ray_t(
    const TOther& o,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    auto tbs = o.bounding_sphere.itransformed(trafo_);
    if (!aabb_intersects_sphere(child_->aabb().casted<ScenePos>(), tbs.template casted<ScenePos>())) {
        return false;
    }
    ScenePos c_overlap;
    FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
    FixedArray<SceneDir, 3> c_normal = uninitialized;
    bool touches = child_->touches(
        o.transformed(trafo_.inverted()),
        c_overlap,
        c_intersection_point,
        c_normal);
    if (!touches) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    intersection_point = trafo_.transform(c_intersection_point);
    normal = trafo_.rotate(c_normal.template casted<float>());
    return true;
}

template <class TOther>
bool TransformedIntersectable::touches_any_with_ray_t(
    const TOther& o,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<SceneDir, 3>& normal) const
{
    auto tbs = o.bounding_sphere.itransformed(trafo_);
    if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
        return false;
    }
    ScenePos c_overlap;
    ScenePos c_ray_t;
    FixedArray<ScenePos, 3> c_intersection_point = uninitialized;
    FixedArray<SceneDir, 3> c_normal = uninitialized;
    bool touches = child_->touches(
        o.transformed(trafo_.inverted()),
        c_overlap,
        c_ray_t,
        c_intersection_point,
        c_normal);
    if (!touches) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    ray_t = (ScenePos)c_ray_t;
    intersection_point = trafo_.transform(c_intersection_point);
    normal = trafo_.rotate(c_normal.template casted<float>());
    return true;
}

template <class TOther>
bool TransformedIntersectable::can_spawn_at_any(const TOther& o) const {
    auto tbs = o.bounding_sphere.itransformed(trafo_);
    if (!aabb_intersects_sphere(child_->aabb().casted<ScenePos>(), tbs.template casted<ScenePos>())) {
        return false;
    }
    return child_->can_spawn_at(o.transformed(trafo_.inverted()));
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
