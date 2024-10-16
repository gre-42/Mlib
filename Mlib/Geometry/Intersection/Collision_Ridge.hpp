#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
static const ScenePos RIDGE_SPECIAL_THRESHOLD = 2.f;
static const ScenePos RIDGE_SINGLE_FACE = 3.f;
static const ScenePos RIDGE_UNTOUCHABLE = 4.f;
static const ScenePos RIDGE_360 = 5.f;

enum class SingleFaceBehavior {
    TOUCHABLE,
    UNTOUCHABLE
};

struct CollisionRidgeSphere {
    BoundingSphere<ScenePos, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<ScenePos, 2, 3> edge;
    RaySegment3D<ScenePos> ray;
    FixedArray<ScenePos, 3> normal;
    FixedArray<float, 2, 3> vertex_normals;
    ScenePos min_cos;
    FixedArray<ScenePos, 3> tangent() const;
    bool is_touchable(SingleFaceBehavior behavior) const;
    bool is_oriented() const;
    void combine(const CollisionRidgeSphere& other, ScenePos max_min_cos_ridge);
    void finalize();
};

struct CollisionEdgeAabb {
    CollisionRidgeSphere base;
    AxisAlignedBoundingBox<ScenePos, 3> aabb;
};

}
