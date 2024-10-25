#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class TData>
struct CollisionLineSphere {
    BoundingSphere<TData, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<TData, 2, 3> line;
    RaySegment3D<TData> ray;
    CollisionLineSphere<ScenePos> transformed(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix) const
    {
        return {
            bounding_sphere.transformed(transformation_matrix),
            physics_material,
            transformation_matrix.transform(line),
            ray.transformed(transformation_matrix),
        };
    }
    template <class TResult>
    CollisionLineSphere<TResult> casted() const {
        return {
            bounding_sphere.template casted<TResult>(),
            physics_material,
            line.template casted<TResult>(),
            ray.template casted<TResult>()
        };
    }
};

template <class TData>
struct CollisionLineAabb {
    CollisionLineSphere<TData> base;
    AxisAlignedBoundingBox<TData, 3> aabb;
};

}
