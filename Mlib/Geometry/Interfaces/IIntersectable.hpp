#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class ScenePos, size_t tndim>
class BoundingSphere;
template <class ScenePos, size_t tndim>
class AxisAlignedBoundingBox;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionRidgeSphere;
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
    virtual bool intersects(
        const CollisionPolygonSphere<4>& q,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionPolygonSphere<3>& t,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionRidgeSphere& r1,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionLineSphere& l1,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
    virtual bool intersects(
        const IIntersectable& intersectable,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
    virtual bool intersects(
        const IIntersectable& intersectable,
        const TransformationMatrix<float, ScenePos, 3>& trafo,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const = 0;
};

}
