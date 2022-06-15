#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>

namespace Mlib {

enum class PhysicsMaterial;

struct CollisionTriangleSphere {
    BoundingSphere<double, 3> bounding_sphere;
    PlaneNd<double, 3> plane;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, 3> triangle;
};

struct CollisionTriangleAabb {
    CollisionTriangleSphere base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
