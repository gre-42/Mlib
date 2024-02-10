#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

struct CollisionLineSphere {
    BoundingSphere<double, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, 2> line;
    RaySegment3D<double> ray;
};

struct CollisionLineAabb {
    CollisionLineSphere base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
