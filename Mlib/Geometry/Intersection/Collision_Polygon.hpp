#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

template <class TData, size_t tnvertices>
struct CollisionPolygonSphere {
    BoundingSphere<TData, 3> bounding_sphere;
    ConvexPolygon3D<TData, tnvertices> polygon;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<TData, 3>, tnvertices> corners;
    inline CollisionPolygonSphere<TData, tnvertices> operator - () const {
        return {
            .bounding_sphere = bounding_sphere,
            .polygon = -polygon,
            .physics_material = physics_material,
            .corners = corners
        };
    }
    CollisionPolygonSphere<ScenePos, tnvertices> transformed(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix) const
    {
        return {
            .bounding_sphere = bounding_sphere.transformed(transformation_matrix),
            .polygon = polygon.template casted<ScenePos>().transformed(transformation_matrix),
            .physics_material = physics_material,
            .corners = corners.template applied<FixedArray<ScenePos, 3>>([&](const auto& c){ return transformation_matrix.transform(c); })
        };
    }
};

template <class TData, size_t tnvertices>
struct CollisionPolygonAabb {
    CollisionPolygonSphere<TData, tnvertices> base;
    AxisAlignedBoundingBox<TData, 3> aabb;
};

}
