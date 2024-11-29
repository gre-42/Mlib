#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

struct CollisionLineSphere {
    BoundingSphere<CompressedScenePos, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<CompressedScenePos, 2, 3> line;
    RaySegment3D<SceneDir, CompressedScenePos> ray;
    CollisionLineSphere transformed(
        const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix) const
    {
        return {
            bounding_sphere.transformed(transformation_matrix),
            physics_material,
            transformation_matrix.transform(line.template casted<ScenePos>()).template casted<CompressedScenePos>(),
            ray.transformed(transformation_matrix),
        };
    }
    // template <class TResult>
    // CollisionLineSphere<TResult> casted() const {
    //     return {
    //         bounding_sphere.template casted<TResult>(),
    //         physics_material,
    //         line.template casted<TResult>(),
    //         ray.template casted<TResult>()
    //     };
    // }
};

struct CollisionLineAabb {
    CollisionLineSphere base;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
