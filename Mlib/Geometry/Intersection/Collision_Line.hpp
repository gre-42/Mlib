#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

struct CollisionLineSphere {
    BoundingSphere<float, 3> bounding_sphere;
    FixedArray<FixedArray<float, 3>, 2> line;
};

struct CollisionLineAabb {
    CollisionLineSphere base;
    AxisAlignedBoundingBox<float, 3> aabb;
};

}
