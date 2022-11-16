#include "Static_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

StaticTransformedMesh::StaticTransformedMesh(
    const std::string& name,
    const AxisAlignedBoundingBox<double, 3>& aabb,
    const BoundingSphere<double, 3>& bounding_sphere,
    std::vector<CollisionTriangleSphere>&& triangles,
    std::vector<CollisionLineSphere>&& lines)
: name_{ name },
  aabb_{ aabb },
  bounding_sphere_{ bounding_sphere },
  triangles_{ std::move(triangles) },
  lines_{ std::move(lines) }
{}

StaticTransformedMesh::~StaticTransformedMesh()
{}

bool StaticTransformedMesh::intersects(const BoundingSphere<double, 3>& sphere) const {
    return bounding_sphere_.intersects(sphere);
}

bool StaticTransformedMesh::intersects(const PlaneNd<double, 3>& plane) const {
    return bounding_sphere_.intersects(plane);
}

const std::vector<CollisionTriangleSphere>& StaticTransformedMesh::get_triangles_sphere() const {
    return triangles_;
}

const std::vector<CollisionLineSphere>& StaticTransformedMesh::get_lines_sphere() const {
    return lines_;
}

BoundingSphere<double, 3> StaticTransformedMesh::bounding_sphere() const {
    return bounding_sphere_;
}

AxisAlignedBoundingBox<double, 3> StaticTransformedMesh::aabb() const {
    return AxisAlignedBoundingBox<double, 3>{
        bounding_sphere_.center(),
        bounding_sphere_.radius()};
}

std::string StaticTransformedMesh::name() const {
    return name_;
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
