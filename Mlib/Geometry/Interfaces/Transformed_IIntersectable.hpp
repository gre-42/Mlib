#pragma once
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <memory>

namespace Mlib {

template <class TData>
class TransformedIntersectable: public IIntersectable<ScenePos> {
    template <class TData2>
    friend class TransformedIntersectable;
public:
    TransformedIntersectable(
        std::shared_ptr<IIntersectable<TData>> child,
        const TransformationMatrix<float, ScenePos, 3>& trafo);
    virtual BoundingSphere<ScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<ScenePos, 3> aabb() const override;
    virtual bool intersects(
        const CollisionPolygonSphere<ScenePos, 4>& q,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
    virtual bool intersects(
        const CollisionPolygonSphere<ScenePos, 3>& t,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
    virtual bool intersects(
        const CollisionRidgeSphere<ScenePos>& r1,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
    virtual bool intersects(
        const CollisionLineSphere<ScenePos>& l1,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
    virtual bool intersects(
        const IIntersectable<ScenePos>& intersectable,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
    virtual bool intersects(
        const IIntersectable<ScenePos>& intersectable,
        const TransformationMatrix<float, ScenePos, 3>& trafo,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const override;
private:
    template <class TOther>
    bool intersects_any_wo_ray_t(
        const TOther& o,
        ScenePos& overlap,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const;
    template <class TOther>
    bool intersects_any_with_ray_t(
        const TOther& o,
        ScenePos& overlap,
        ScenePos& ray_t,
        FixedArray<ScenePos, 3>& intersection_point,
        FixedArray<ScenePos, 3>& normal) const;
    std::shared_ptr<IIntersectable<TData>> child_;
    TransformationMatrix<float, ScenePos, 3> trafo_;
};

}
