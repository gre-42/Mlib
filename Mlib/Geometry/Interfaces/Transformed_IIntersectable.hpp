#pragma once
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

class TransformedIntersectable: public IIntersectable {
public:
    TransformedIntersectable(
        std::shared_ptr<IIntersectable> child,
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo);
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const override;
    virtual std::shared_ptr<IIntersectable> sweep(
        const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool touches(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool touches(
        const CollisionRidgeSphere<CompressedScenePos>& r1,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool touches(
        const CollisionLineSphere<CompressedScenePos>& l1,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool touches(
        const IIntersectable& intersectable,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool touches(
        const IIntersectable& intersectable,
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const override;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 3>& t) const override;
    virtual bool can_spawn_at(
        const CollisionPolygonSphere<CompressedScenePos, 4>& q) const override;
    virtual bool can_spawn_at(
        const IIntersectable& intersectable) const override;
    virtual bool can_spawn_at(
        const IIntersectable& intersectable,
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const override;
private:
    template <class TOther>
    bool touches_any_wo_ray_t(
        const TOther& o,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    template <class TOther>
    bool touches_any_with_ray_t(
        const TOther& o,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<SceneDir, 3>& normal) const;
    template <class TOther>
    bool can_spawn_at_any(const TOther& o) const;
    std::shared_ptr<IIntersectable> child_;
    TransformationMatrix<SceneDir, ScenePos, 3> trafo_;
};

}
