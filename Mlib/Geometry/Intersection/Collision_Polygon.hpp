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

template <size_t tnvertices>
struct CollisionPolygonSphere {
    BoundingSphere<CompressedScenePos, 3> bounding_sphere;
    ConvexPolygon3D<SceneDir, CompressedScenePos, tnvertices> polygon;
    PhysicsMaterial physics_material;
    FixedArray<CompressedScenePos, tnvertices, 3> corners;
    inline CollisionPolygonSphere<tnvertices> operator - () const {
        return {
            .bounding_sphere = bounding_sphere,
            .polygon = -polygon,
            .physics_material = physics_material,
            .corners = corners
        };
    }
    CollisionPolygonSphere<tnvertices> transformed(
        const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix) const
    {
        return {
            .bounding_sphere = bounding_sphere.transformed(transformation_matrix),
            .polygon = polygon
                .template casted<SceneDir, ScenePos>()
                .transformed(transformation_matrix)
                .template casted<SceneDir, CompressedScenePos>(),
            .physics_material = physics_material,
            .corners = transformation_matrix
                .transform(corners.template casted<ScenePos>())
                .template casted<CompressedScenePos>()
        };
    }
    // template <class TResult>
    // CollisionPolygonSphere<TResult, tnvertices> casted() const {
    //     return {
    //         bounding_sphere.template casted<TResult>(),
    //         polygon.template casted<TResult>(),
    //         physics_material,
    //         corners.template casted<TResult>()
    //     };
    // }
};

template <size_t tnvertices>
struct CollisionPolygonAabb {
    CollisionPolygonSphere<tnvertices> base;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
