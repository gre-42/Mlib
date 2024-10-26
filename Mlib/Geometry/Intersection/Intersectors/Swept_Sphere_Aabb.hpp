#pragma once
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

template <class TData>
class SweptSphereAabb: public IIntersectable<TData> {
public:
    SweptSphereAabb(
        const FixedArray<TData, 3>& min,
        const FixedArray<TData, 3>& max,
        const TData& radius);
    virtual BoundingSphere<TData, 3> bounding_sphere() const;
    virtual bool intersects(
        const CollisionPolygonSphere<TData, 4>& q,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
    virtual bool intersects(
        const CollisionPolygonSphere<TData, 3>& t,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
    virtual bool intersects(
        const CollisionRidgeSphere<TData>& r1,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
    virtual bool intersects(
        const CollisionLineSphere<TData>& l1,
        TData& overlap,
        TData& ray_t,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
    virtual bool intersects(
        const IIntersectable<TData>& intersectable,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
    virtual bool intersects(
        const IIntersectable<TData>& intersectable,
        const TransformationMatrix<float, TData, 3>& trafo,
        TData& overlap,
        FixedArray<TData, 3>& intersection_point,
        FixedArray<TData, 3>& normal) const;
private:
    AxisAlignedBoundingBox<TData, 3> aabb_;
    TData radius_;
    BoundingSphere<TData, 3> bounding_sphere_;
};

}
