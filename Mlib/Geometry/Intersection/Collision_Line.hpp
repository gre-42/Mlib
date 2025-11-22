#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstdint>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class TPosition>
struct CollisionLineSphere {
    BoundingSphere<TPosition, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<TPosition, 2, 3> line;
    RaySegment3D<SceneDir, TPosition> ray;
    CollisionLineSphere transformed(
        const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix) const
    {
        return {
            bounding_sphere.transformed(transformation_matrix),
            physics_material,
            transformation_matrix.transform(line.template casted<ScenePos>()).template casted<TPosition>(),
            ray.transformed(transformation_matrix),
        };
    }
    template <class TPosition2>
    inline CollisionLineSphere<TPosition2> casted() const {
        return {
            bounding_sphere.template casted<TPosition2>(),
            physics_material,
            line.template casted<TPosition2>(),
            ray.template casted<SceneDir, TPosition2>()
        };
    }
    bool operator == (const CollisionLineSphere& other) const {
        return (bounding_sphere == other.bounding_sphere) &&
               (physics_material == other.physics_material) &&
               all(line == other.line) &&
               (ray == other.ray);
    }
};

template <class TPosition>
CollisionLineSphere<TPosition> operator + (
    const CollisionLineSphere<TPosition>& clp,
    const FixedArray<TPosition, 3>& p)
{
    return {
        clp.bounding_sphere + p,
        clp.physics_material,
        FixedArray<TPosition, 2, 3>{ clp.line[0] + p, clp.line[1] + p },
        clp.ray + p
    };
}

template <class TPosition>
CollisionLineSphere<TPosition> operator - (
    const CollisionLineSphere<TPosition>& clp,
    const FixedArray<TPosition, 3>& p)
{
    return clp + (-p);
}

template <class TPosition>
struct CollisionLineAabb {
    CollisionLineSphere<TPosition> base;
    AxisAlignedBoundingBox<TPosition, 3> aabb;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
