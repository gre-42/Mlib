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
    std::string name,
    const AxisAlignedBoundingBox<ScenePos, 3>& aabb,
    const BoundingSphere<ScenePos, 3>& bounding_sphere,
    std::vector<CollisionPolygonSphere<ScenePos, 4>>&& quads,
    std::vector<CollisionPolygonSphere<ScenePos, 3>>&& triangles,
    std::vector<CollisionLineSphere<ScenePos>>&& lines,
    std::vector<CollisionLineSphere<ScenePos>>&& edges,
    std::vector<CollisionRidgeSphere>&& ridges)
: name_{ std::move(name) },
  aabb_{ aabb },
  bounding_sphere_{ bounding_sphere },
  quads_{ std::move(quads) },
  triangles_{ std::move(triangles) },
  lines_{ std::move(lines) },
  edges_{ std::move(edges) },
  ridges_{ std::move(ridges) }
{}

StaticTransformedMesh::~StaticTransformedMesh() = default;

bool StaticTransformedMesh::intersects(const BoundingSphere<ScenePos, 3>& sphere) const {
    return bounding_sphere_.intersects(sphere);
}

bool StaticTransformedMesh::intersects(const PlaneNd<ScenePos, 3>& plane) const {
    return bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<ScenePos, 4>>& StaticTransformedMesh::get_quads_sphere() const {
    return quads_;
}

const std::vector<CollisionPolygonSphere<ScenePos, 3>>& StaticTransformedMesh::get_triangles_sphere() const {
    return triangles_;
}

const std::vector<CollisionLineSphere<ScenePos>>& StaticTransformedMesh::get_lines_sphere() const {
    return lines_;
}

const std::vector<CollisionLineSphere<ScenePos>>& StaticTransformedMesh::get_edges_sphere() const {
    return edges_;
}

const std::vector<CollisionRidgeSphere>& StaticTransformedMesh::get_ridges_sphere() const {
    return ridges_;
}

BoundingSphere<ScenePos, 3> StaticTransformedMesh::bounding_sphere() const {
    return bounding_sphere_;
}

AxisAlignedBoundingBox<ScenePos, 3> StaticTransformedMesh::aabb() const {
    return aabb_;
}

std::string StaticTransformedMesh::name() const {
    return name_;
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
