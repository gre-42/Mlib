#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
static const float RIDGE_SPECIAL_THRESHOLD = 2.f;
static const float RIDGE_SINGLE_FACE = 3.f;
static const float RIDGE_UNTOUCHABLE = 4.f;
static const float RIDGE_360 = 5.f;

enum class SingleFaceBehavior {
    TOUCHABLE,
    UNTOUCHABLE
};

struct CollisionRidgeSphere {
    BoundingSphere<CompressedScenePos, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<CompressedScenePos, 2, 3> edge;
    RaySegment3D<SceneDir, CompressedScenePos> ray;
    FixedArray<ScenePos, 3> normal;
    float min_cos;
    FixedArray<ScenePos, 3> tangent() const;
    bool is_touchable(SingleFaceBehavior behavior) const;
    bool is_oriented() const;
    void combine(const CollisionRidgeSphere& other, float max_min_cos_ridge);
    void finalize();
    CollisionRidgeSphere transformed(const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const;
};

struct CollisionEdgeAabb {
    CollisionRidgeSphere base;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb;
};

}
