#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>

namespace Mlib {

struct CollisionTriangleSphere {
    BoundingSphere<float, 3> bounding_sphere;
    PlaneNd<float, 3> plane;
    bool two_sided;
    FixedArray<FixedArray<float, 3>, 3> triangle;
};

struct CollisionTriangleAabb {
    CollisionTriangleSphere base;
    AxisAlignedBoundingBox<float, 3> aabb;
};

}
