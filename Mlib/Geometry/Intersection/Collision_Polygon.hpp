#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>

namespace Mlib {

enum class PhysicsMaterial;

template <size_t tnvertices>
struct CollisionPolygonSphere {
    BoundingSphere<double, 3> bounding_sphere;
    ConvexPolygon3D<double, tnvertices> polygon;
    PhysicsMaterial physics_material;
    FixedArray<FixedArray<double, 3>, tnvertices> corners;
    inline CollisionPolygonSphere<tnvertices> operator - () const {
        return {
            .bounding_sphere = bounding_sphere,
            .polygon = -polygon,
            .physics_material = physics_material,
            .corners = corners
        };
    }
};

template <size_t tnvertices>
struct CollisionPolygonAabb {
    CollisionPolygonSphere<tnvertices> base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
