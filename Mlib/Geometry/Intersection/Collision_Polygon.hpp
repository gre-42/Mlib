#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere {
    BoundingSphere<TPosition, 3> bounding_sphere;
    ConvexPolygon3D<SceneDir, TPosition, tnvertices> polygon;
    PhysicsMaterial physics_material;
    FixedArray<TPosition, tnvertices, 3> corners;
    inline CollisionPolygonSphere<TPosition, tnvertices> operator - () const {
        return {
            .bounding_sphere = bounding_sphere,
            .polygon = -polygon,
            .physics_material = physics_material,
            .corners = corners
        };
    }
    CollisionPolygonSphere<TPosition, tnvertices> transformed(
        const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix) const
    {
        return {
            .bounding_sphere = bounding_sphere.transformed(transformation_matrix),
            .polygon = polygon
                .template casted<SceneDir, ScenePos>()
                .transformed(transformation_matrix)
                .template casted<SceneDir, TPosition>(),
            .physics_material = physics_material,
            .corners = transformation_matrix
                .transform(corners.template casted<ScenePos>())
                .template casted<TPosition>()
        };
    }
    template <class TResult>
    inline CollisionPolygonSphere<TResult, tnvertices> casted() const {
        return {
            bounding_sphere.template casted<TResult>(),
            polygon.template casted<SceneDir, TResult>(),
            physics_material,
            corners.template casted<TResult>()
        };
    }
    bool operator == (const CollisionPolygonSphere& other) const {
        return (bounding_sphere == other.bounding_sphere) &&
               (polygon == other.polygon) &&
               (physics_material == other.physics_material) &&
               all(corners == other.corners);
    }
};

template <class TPosition, size_t tnvertices>
CollisionPolygonSphere<TPosition, tnvertices>
    operator + (
        const CollisionPolygonSphere<TPosition, tnvertices>& a,
        const FixedArray<TPosition, 3>& d)
{
    auto corners = a.corners;
    for (size_t r = 0; r < tnvertices; ++r) {
        corners[r] += d;
    }
    return {
        a.bounding_sphere + d,
        a.polygon + d,
        a.physics_material,
        corners};
}

template <class TPosition, size_t tnvertices>
CollisionPolygonSphere<TPosition, tnvertices>
    operator - (
        const CollisionPolygonSphere<TPosition, tnvertices>& a,
        const FixedArray<TPosition, 3>& d)
{
    return a + (-d);
}

template <class TPosition, size_t tnvertices>
struct CollisionPolygonAabb {
    CollisionPolygonSphere<TPosition, tnvertices> base;
    AxisAlignedBoundingBox<TPosition, 3> aabb;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
