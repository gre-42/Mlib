#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

enum class PhysicsMaterial;

struct CollisionRidgeSphere {
    BoundingSphere<double, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, 2> edge;
    FixedArray<double, 3> normal;
    double min_cos;
};

struct CollisionEdgeAabb {
    CollisionRidgeSphere base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
