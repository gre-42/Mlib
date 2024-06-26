#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
static const double RIDGE_SPECIAL_THRESHOLD = 2.;
static const double RIDGE_SINGLE_FACE = 3.;
static const double RIDGE_UNTOUCHABLE = 4.;
static const double RIDGE_360 = 5.;

enum class SingleFaceBehavior {
    TOUCHABLE,
    UNTOUCHABLE
};

struct CollisionRidgeSphere {
    BoundingSphere<double, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, 2> edge;
    RaySegment3D<double> ray;
    FixedArray<double, 3> normal;
    double min_cos;
    FixedArray<double, 3> tangent() const;
    bool is_touchable(SingleFaceBehavior behavior) const;
    bool is_oriented() const;
    void combine(const CollisionRidgeSphere& other, double max_min_cos_ridge);
    void finalize();
};

struct CollisionEdgeAabb {
    CollisionRidgeSphere base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
