#pragma once
#include <cstddef>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
struct CollisionRidgeSphere;
template <class TData>
struct CollisionLineSphere;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData>
class IIntersectable {
public:
    virtual ~IIntersectable() = default;
    virtual BoundingSphere<TData, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<TData, 3> aabb() const = 0;
    virtual bool intersects(
        const CollisionPolygonSphere<TData, 4>& q,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionPolygonSphere<TData, 3>& t,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionRidgeSphere<TData>& r1,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
    virtual bool intersects(
        const CollisionLineSphere<TData>& l1,
        TData& overlap,
        TData& ray_t,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
    virtual bool intersects(
        const IIntersectable<TData>& intersectable,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
    virtual bool intersects(
        const IIntersectable<TData>& intersectable,
        const TransformationMatrix<float, TData, 3>& trafo,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const = 0;
};

}
