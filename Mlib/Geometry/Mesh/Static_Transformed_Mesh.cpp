#include "Static_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Primitives/Collision_Line.hpp>
#include <Mlib/Geometry/Primitives/Collision_Polygon.hpp>
#include <Mlib/Geometry/Primitives/Plane_Nd.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>

using namespace Mlib;

PRAGMA_GCC(push_options)
PRAGMA_GCC(optimize ("O3"))

StaticTransformedMesh::StaticTransformedMesh(
    std::string name,
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb,
    const BoundingSphere<CompressedScenePos, 3>& bounding_sphere,
    std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>&& quads,
    std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>&& triangles,
    std::vector<CollisionLineSphere<CompressedScenePos>>&& lines,
    std::vector<CollisionLineSphere<CompressedScenePos>>&& edges,
    std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables)
    : name_{ std::move(name) }
    , aabb_{ aabb }
    , bounding_sphere_{ bounding_sphere }
    , quads_{ std::move(quads) }
    , triangles_{ std::move(triangles) }
    , lines_{ std::move(lines) }
    , edges_{ std::move(edges) }
    , intersectables_{ std::move(intersectables) }
{}

StaticTransformedMesh::~StaticTransformedMesh() = default;

bool StaticTransformedMesh::intersects(const BoundingSphere<CompressedScenePos, 3>& sphere) const {
    return bounding_sphere_.intersects(sphere);
}

bool StaticTransformedMesh::intersects(const PlaneNd<SceneDir, CompressedScenePos, 3>& plane) const {
    return bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& StaticTransformedMesh::get_quads_sphere() const {
    return quads_;
}

const std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& StaticTransformedMesh::get_triangles_sphere() const {
    return triangles_;
}

const std::vector<CollisionLineSphere<CompressedScenePos>>& StaticTransformedMesh::get_lines_sphere() const {
    return lines_;
}

const std::vector<CollisionLineSphere<CompressedScenePos>>& StaticTransformedMesh::get_edges_sphere() const {
    return edges_;
}

const std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>& StaticTransformedMesh::get_intersectables() const
{
    return intersectables_;
}

BoundingSphere<CompressedScenePos, 3> StaticTransformedMesh::bounding_sphere() const {
    return bounding_sphere_;
}

AxisAlignedBoundingBox<CompressedScenePos, 3> StaticTransformedMesh::aabb() const {
    return aabb_;
}

std::string StaticTransformedMesh::name() const {
    return name_;
}

PRAGMA_GCC(pop_options)
