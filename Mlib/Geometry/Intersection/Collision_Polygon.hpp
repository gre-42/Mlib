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
};

template <size_t tnvertices>
struct CollisionPolygonAabb {
    CollisionPolygonSphere<tnvertices> base;
    AxisAlignedBoundingBox<double, 3> aabb;
};

}
