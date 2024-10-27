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

template <class TData>
TransformedIntersectable<TData>::TransformedIntersectable(
    std::shared_ptr<IIntersectable<TData>> child,
    const TransformationMatrix<float, ScenePos, 3>& trafo)
    : child_{ child }
    , trafo_{ trafo }
{}

template <class TData>
BoundingSphere<ScenePos, 3> TransformedIntersectable<TData>::bounding_sphere() const {
    return child_->bounding_sphere().transformed(trafo_);
}

template <class TData>
AxisAlignedBoundingBox<ScenePos, 3> TransformedIntersectable<TData>::aabb() const {
    return child_->aabb().template casted<ScenePos>().transformed(trafo_);
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const CollisionPolygonSphere<ScenePos, 4>& q,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    return intersects_any_wo_ray_t(q, overlap, intersection_point, normal);
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const CollisionPolygonSphere<ScenePos, 3>& t,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    return intersects_any_wo_ray_t(t, overlap, intersection_point, normal);
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const CollisionRidgeSphere<ScenePos>& r1,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    return intersects_any_wo_ray_t(r1, overlap, intersection_point, normal);
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const CollisionLineSphere<ScenePos>& l1,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    return intersects_any_with_ray_t(l1, overlap, ray_t, intersection_point, normal);
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const IIntersectable<ScenePos>& intersectable,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    auto* o = dynamic_cast<const TransformedIntersectable<TData>*>(&intersectable);
    if (o == nullptr) {
        THROW_OR_ABORT("TransformedIntersectable can only intersect children of the same type");
    }
    TData c_overlap;
    FixedArray<TData, 3> c_intersection_point = uninitialized;
    FixedArray<TData, 3> c_normal = uninitialized;
    bool intersects = child_->intersects(
        *o->child_,
        (trafo_.inverted() * o->trafo_).template casted<float, TData>(),
        c_overlap,
        c_intersection_point,
        c_normal);
    if (!intersects) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    intersection_point = trafo_.transform(c_intersection_point.template casted<float>()).template casted<ScenePos>();
    normal = trafo_.rotate(c_normal.template casted<float>()).template casted<ScenePos>();
    return true;
}

template <class TData>
bool TransformedIntersectable<TData>::intersects(
    const IIntersectable<ScenePos>& intersectable,
    const TransformationMatrix<float, ScenePos, 3>& trafo,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    THROW_OR_ABORT("TransformedIIntersectable received additional transformation matrix");
}

template <class TData>
template <class TOther>
bool TransformedIntersectable<TData>::intersects_any_wo_ray_t(
    const TOther& o,
    ScenePos& overlap,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    auto tbs = o.bounding_sphere.itransformed(trafo_).template casted<TData>();
    if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
        return false;
    }
    TData c_overlap;
    FixedArray<TData, 3> c_intersection_point = uninitialized;
    FixedArray<TData, 3> c_normal = uninitialized;
    bool intersects = child_->intersects(
        o.transformed(trafo_.inverted()).template casted<TData>(),
        c_overlap,
        c_intersection_point,
        c_normal);
    if (!intersects) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    intersection_point = trafo_.transform(c_intersection_point.template casted<float>()).template casted<ScenePos>();
    normal = trafo_.rotate(c_normal.template casted<float>()).template casted<ScenePos>();
    return true;
}

template <class TData>
template <class TOther>
bool TransformedIntersectable<TData>::intersects_any_with_ray_t(
    const TOther& o,
    ScenePos& overlap,
    ScenePos& ray_t,
    FixedArray<ScenePos, 3>& intersection_point,
    FixedArray<ScenePos, 3>& normal) const
{
    auto tbs = o.bounding_sphere.itransformed(trafo_).template casted<TData>();
    if (!aabb_intersects_sphere(child_->aabb(), tbs)) {
        return false;
    }
    TData c_overlap;
    TData c_ray_t;
    FixedArray<TData, 3> c_intersection_point = uninitialized;
    FixedArray<TData, 3> c_normal = uninitialized;
    bool intersects = child_->intersects(
        o.transformed(trafo_.inverted()).template casted<TData>(),
        c_overlap,
        c_ray_t,
        c_intersection_point,
        c_normal);
    if (!intersects) {
        return false;
    }
    overlap = (ScenePos)c_overlap;
    ray_t = (ScenePos)c_ray_t;
    intersection_point = trafo_.transform(c_intersection_point.template casted<float>()).template casted<ScenePos>();
    normal = trafo_.rotate(c_normal.template casted<float>()).template casted<ScenePos>();
    return true;
}

namespace Mlib {

template class TransformedIntersectable<double>;
template class TransformedIntersectable<float>;

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
