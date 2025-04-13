#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class ScenePos, size_t tndim>
class BoundingSphere;
template <class ScenePos, size_t tndim>
class AxisAlignedBoundingBox;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition>
struct CollisionRidgeSphere;
template <class TPosition>
struct CollisionLineSphere;
template <typename ScenePos, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IIntersectable {
public:
    virtual ~IIntersectable() = default;
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const = 0;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool touches(
        const CollisionRidgeSphere<CompressedScenePos>& r1,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool touches(
        const CollisionLineSphere<CompressedScenePos>& l1,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool touches(
        const IIntersectable& intersectable,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool touches(
        const IIntersectable& intersectable,
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const = 0;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t) const = 0;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q) const = 0;
    virtual bool can_spawn_at(
        const IIntersectable& intersectable) const = 0;
    virtual bool can_spawn_at(
        const IIntersectable& intersectable,
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const = 0;
};

}
