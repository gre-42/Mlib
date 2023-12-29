#include "Static_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
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
    std::vector<CollisionPolygonSphere<4>>&& quads,
    std::vector<CollisionPolygonSphere<3>>&& triangles,
    std::vector<CollisionLineSphere>&& lines,
    std::vector<CollisionLineSphere>&& edges,
    std::vector<CollisionRidgeSphere>&& ridges)
: name_{ name },
  aabb_{ aabb },
  bounding_sphere_{ bounding_sphere },
  quads_{ std::move(quads) },
  triangles_{ std::move(triangles) },
  lines_{ std::move(lines) },
  edges_{ std::move(edges) },
  ridges_{ std::move(ridges) }
{}

StaticTransformedMesh::~StaticTransformedMesh()
{}

bool StaticTransformedMesh::intersects(const BoundingSphere<double, 3>& sphere) const {
    return bounding_sphere_.intersects(sphere);
}

bool StaticTransformedMesh::intersects(const PlaneNd<double, 3>& plane) const {
    return bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<4>>& StaticTransformedMesh::get_quads_sphere() const {
    return quads_;
}

const std::vector<CollisionPolygonSphere<3>>& StaticTransformedMesh::get_triangles_sphere() const {
    return triangles_;
}

const std::vector<CollisionLineSphere>& StaticTransformedMesh::get_lines_sphere() const {
    return lines_;
}

const std::vector<CollisionLineSphere>& StaticTransformedMesh::get_edges_sphere() const {
    return edges_;
}

const std::vector<CollisionRidgeSphere>& StaticTransformedMesh::get_ridges_sphere() const {
    return ridges_;
}

BoundingSphere<double, 3> StaticTransformedMesh::bounding_sphere() const {
    return bounding_sphere_;
}

AxisAlignedBoundingBox<double, 3> StaticTransformedMesh::aabb() const {
    return aabb_;
}

std::string StaticTransformedMesh::name() const {
    return name_;
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
