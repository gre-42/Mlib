#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
static const SceneDir RIDGE_SPECIAL_THRESHOLD = 2.f;
static const SceneDir RIDGE_SINGLE_FACE = 3.f;
static const SceneDir RIDGE_UNTOUCHABLE = 4.f;
static const SceneDir RIDGE_360 = 5.f;

enum class SingleFaceBehavior {
    TOUCHABLE,
    UNTOUCHABLE
};

template <class TPosition>
struct CollisionRidgeSphere {
    BoundingSphere<TPosition, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<TPosition, 2, 3> edge;
    RaySegment3D<SceneDir, TPosition> ray;
    FixedArray<SceneDir, 3> normal;
    SceneDir min_cos;
    FixedArray<SceneDir, 3> tangent() const;
    bool is_touchable(SingleFaceBehavior behavior) const;
    bool is_oriented() const;
    void combine(const CollisionRidgeSphere<TPosition>& other, SceneDir max_min_cos_ridge);
    void finalize();
    CollisionRidgeSphere transformed(const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const;
    template <class TPosition2>
    inline CollisionRidgeSphere<TPosition2> casted() const {
        return CollisionRidgeSphere<TPosition2>{
            .bounding_sphere = bounding_sphere.template casted<TPosition2>(),
            .physics_material = physics_material,
            .edge = edge.template casted<TPosition2>(),
            .ray = ray.template casted<SceneDir, TPosition2>(),
            .normal = normal,
            .min_cos = min_cos
        };
    }
    bool operator == (const CollisionRidgeSphere& other) const {
        return (bounding_sphere == other.bounding_sphere) &&
               (physics_material == other.physics_material) &&
               all(edge == other.edge) &&
               (ray == other.ray) &&
               all(normal == other.normal) &&
               (min_cos == other.min_cos);
    }
};

template <class TPosition>
CollisionRidgeSphere<TPosition> operator + (
    const CollisionRidgeSphere<TPosition>& crs,
    const FixedArray<TPosition, 3>& p)
{
    return {
        crs.bounding_sphere + p,
        crs.physics_material,
        FixedArray<TPosition, 2, 3>{ crs.edge[0] + p,  crs.edge[1] + p },
        crs.ray + p,
        crs.normal,
        crs.min_cos
    };
}

}
