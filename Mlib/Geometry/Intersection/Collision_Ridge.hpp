#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Pos.hpp>
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

template <class TData>
struct CollisionRidgeSphere {
    BoundingSphere<TData, 3> bounding_sphere;
    PhysicsMaterial physics_material;
    FixedArray<TData, 2, 3> edge;
    RaySegment3D<TData> ray;
    FixedArray<TData, 3> normal;
    TData min_cos;
    FixedArray<TData, 3> tangent() const;
    bool is_touchable(SingleFaceBehavior behavior) const;
    bool is_oriented() const;
    void combine(const CollisionRidgeSphere& other, TData max_min_cos_ridge);
    void finalize();
    template <class TResult>
    CollisionRidgeSphere<TResult> transformed(const TransformationMatrix<float, TResult, 3>& trafo) const;
    template <class TResult>
    CollisionRidgeSphere<TResult> casted() const;
};

template <class TData>
struct CollisionEdgeAabb {
    CollisionRidgeSphere<TData> base;
    AxisAlignedBoundingBox<TData, 3> aabb;
};

}
