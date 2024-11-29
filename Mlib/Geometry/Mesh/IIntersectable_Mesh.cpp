#include "IIntersectable_Mesh.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/Collision_Vertices.hpp>
#include <mutex>
#include <shared_mutex>

using namespace Mlib;

IIntersectableMesh::IIntersectableMesh() = default;

IIntersectableMesh::~IIntersectableMesh() = default;

bool IIntersectableMesh::intersects(const IIntersectableMesh& other) const {
    return intersects(other.bounding_sphere());
}

const std::set<OrderableFixedArray<CompressedScenePos, 3>>& IIntersectableMesh::get_vertices() const {
    {
        std::shared_lock lock{ mutex_ };
        if (collision_vertices_ != nullptr) {
            return collision_vertices_->get();
        }
    }
    std::scoped_lock lock{ mutex_ };
    if (collision_vertices_ != nullptr) {
        return collision_vertices_->get();
    }
    collision_vertices_ = std::make_unique<CollisionVertices>();
    for (const auto& e : get_quads_sphere()) {
        collision_vertices_->insert(e.corners);
    }
    for (const auto& e : get_triangles_sphere()) {
        collision_vertices_->insert(e.corners);
    }
    for (const auto& e : get_lines_sphere()) {
        collision_vertices_->insert(e.line);
    }
    for (const auto& e : get_edges_sphere()) {
        collision_vertices_->insert(e.line);
    }
    for (const auto& e : get_ridges_sphere()) {
        collision_vertices_->insert(e.edge);
    }
    return collision_vertices_->get();
}
