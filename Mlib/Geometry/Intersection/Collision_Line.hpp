#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class TData>
struct CollisionLineSphere {
    BoundingSphere<TData, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<TData, 3>, 2> line;
    RaySegment3D<TData> ray;
    CollisionLineSphere<double> transformed(
        const TransformationMatrix<float, double, 3>& transformation_matrix) const
    {
        return {
            bounding_sphere.transformed(transformation_matrix),
            physics_material,
            {
                transformation_matrix.transform(line(0)),
                transformation_matrix.transform(line(1))
            },
            ray.transformed(transformation_matrix),
        };
    }
};

template <class TData>
struct CollisionLineAabb {
    CollisionLineSphere<TData> base;
    AxisAlignedBoundingBox<TData, 3> aabb;
};

}
