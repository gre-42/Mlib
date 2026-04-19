#include "Lazy_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Interfaces/Transformed_IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Collision_Edges.hpp>
#include <Mlib/Geometry/Mesh/Collision_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Primitives/Collision_Line.hpp>
#include <Mlib/Geometry/Primitives/Collision_Polygon.hpp>
#include <Mlib/Geometry/Primitives/Plane_Nd.hpp>
#include <Mlib/Geometry/Primitives/Quad_3D.hpp>
#include <Mlib/Geometry/Primitives/Triangle_3D.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>

using namespace Mlib;

PRAGMA_GCC(push_options)
PRAGMA_GCC(optimize ("O3"))

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix,
    const BoundingSphere<CompressedScenePos, 3>& bounding_sphere,
    const std::shared_ptr<CollisionMesh>& collision_mesh)
    : transformation_matrix_{ transformation_matrix }
    , transformed_bounding_sphere_{ bounding_sphere.transformed(transformation_matrix) }
    , mesh_{ collision_mesh }
{}

LazyTransformedMesh::~LazyTransformedMesh() = default;

bool LazyTransformedMesh::intersects(const BoundingSphere<CompressedScenePos, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool LazyTransformedMesh::intersects(const PlaneNd<SceneDir, CompressedScenePos, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& LazyTransformedMesh::get_quads_sphere() const {
    if (!quads_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!quads_calculated_) {
            transformed_quads_.reserve(mesh_->quads.size());
            for (const auto& q : mesh_->quads) {
                transformed_quads_.push_back(q.transformed(transformation_matrix_));
            }
            quads_calculated_ = true;
        }
    }
    return transformed_quads_;
}


const std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& LazyTransformedMesh::get_triangles_sphere() const {
    if (!triangles_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!triangles_calculated_) {
            transformed_triangles_.reserve(mesh_->triangles.size());
            for (const auto& t : mesh_->triangles) {
                transformed_triangles_.push_back(t.transformed(transformation_matrix_));
            }
            triangles_calculated_ = true;
        }
    }
    return transformed_triangles_;
}

const std::vector<CollisionLineSphere<CompressedScenePos>>& LazyTransformedMesh::get_edges_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    lerr() << "Skipping mesh without triangles";
    //}
    if (!edges_calculated_) {
        get_quads_sphere();
        get_triangles_sphere();
        std::scoped_lock lock{mutex_};
        if (!edges_calculated_) {
            CollisionEdges edges;
            for (const auto& q3 : transformed_quads_) {
                edges.insert(q3.corners, q3.physics_material);
            }
            for (const auto& t3 : transformed_triangles_) {
                edges.insert(t3.corners, t3.physics_material);
            }
            transformed_edges_.reserve(edges.size());
            for (const auto& e : edges) {
                transformed_edges_.push_back(e.collision_line_sphere);
            }
            edges_calculated_ = true;
        }
    }
    return transformed_edges_;
}

const std::vector<CollisionLineSphere<CompressedScenePos>>& LazyTransformedMesh::get_lines_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    lerr() << "Skipping mesh without triangles";
    //}
    if (!lines_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!lines_calculated_) {
            transformed_lines_.reserve(mesh_->lines.size());
            for (const auto& l2 : mesh_->lines) {
                transformed_lines_.push_back(l2.transformed(transformation_matrix_));
            }
            lines_calculated_ = true;
        }
    }
    return transformed_lines_;
}

const std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>& LazyTransformedMesh::get_intersectables() const
{
    if (!intersectables_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!intersectables_calculated_) {
            transformed_intersectables_.reserve(
                ((mesh_ == nullptr) || (mesh_->intersectable.mesh == nullptr) ? 0 : 1));
            if ((mesh_ != nullptr) && (mesh_->intersectable.mesh != nullptr)) {
                transformed_intersectables_.emplace_back(
                    mesh_->intersectable.physics_material,
                    std::make_shared<TransformedIntersectable>(
                        mesh_->intersectable.mesh,
                        transformation_matrix_));
            }
            intersectables_calculated_ = true;
        }
    }
    return transformed_intersectables_;
}

BoundingSphere<CompressedScenePos, 3> LazyTransformedMesh::bounding_sphere() const {
    return transformed_bounding_sphere_;
}

AxisAlignedBoundingBox<CompressedScenePos, 3> LazyTransformedMesh::aabb() const {
    return AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(
        transformed_bounding_sphere_.center,
        transformed_bounding_sphere_.radius);
}

PRAGMA_GCC(pop_options)

void LazyTransformedMesh::print_info() const {
    lerr() << "LazyTransformedMesh";
    lerr() << transformed_bounding_sphere_.center;
    lerr() << transformed_bounding_sphere_.radius;
}

std::string LazyTransformedMesh::name() const {
    return mesh_->name;
}
