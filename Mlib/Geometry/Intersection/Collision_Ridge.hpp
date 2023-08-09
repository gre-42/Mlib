#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

namespace Mlib {

enum class PhysicsMaterial;
static const double RIDGE_SPECIAL_THRESHOLD = 2.f;
static const double RIDGE_SINGLE_FACE = 3.;
static const double RIDGE_UNTOUCHEABLE = 4.;
static const double RIDGE_360 = 5.;

struct CollisionRidgeSphere {
    BoundingSphere<double, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, 2> edge;
    FixedArray<double, 3> normal;
    double min_cos;
    inline bool is_oriented() const {
        if (min_cos == RIDGE_SINGLE_FACE) {
            THROW_OR_ABORT("CollisionRidgeSphere has not been finalized");
        }
        if (min_cos == RIDGE_UNTOUCHEABLE) {
            THROW_OR_ABORT("Collision attempt with untouchable CollisionRidgeSphere");
        }
        return min_cos < RIDGE_SPECIAL_THRESHOLD;
    }
    void combine(const CollisionRidgeSphere& other, double max_min_cos_ridge);
    void finalize();
};

struct CollisionEdgeAabb {
    CollisionRidgeSphere base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
