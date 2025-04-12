#pragma once
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

class SweptSphereAabb: public IIntersectable {
public:
    SweptSphereAabb(
        const FixedArray<CompressedScenePos, 3>& min,
        const FixedArray<CompressedScenePos, 3>& max,
        const CompressedScenePos& radius);
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool touches(
        const CollisionRidgeSphere<CompressedScenePos>& r1,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool touches(
        const CollisionLineSphere<CompressedScenePos>& l1,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool touches(
        const IIntersectable& intersectable,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool touches(
        const IIntersectable& intersectable,
        const TransformationMatrix<float, ScenePos, 3>& trafo,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t) const override;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q) const override;
private:
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb_small_;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb_large_;
    CompressedScenePos radius_;
    BoundingSphere<CompressedScenePos, 3> bounding_sphere_;
};

}
