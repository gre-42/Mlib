#include "Lazy_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/Collision_Edges.hpp>
#include <Mlib/Geometry/Mesh/Collision_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Geometry/Quad_3D.hpp>
#include <Mlib/Geometry/Triangle_3D.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
    const BoundingSphere<float, 3>& bounding_sphere,
    const std::shared_ptr<CollisionMesh<float>>& collision_mesh,
    ScenePos max_min_cos_ridge)
    : max_min_cos_ridge_{max_min_cos_ridge}
    , transformation_matrix_{ transformation_matrix }
    , transformed_bounding_sphere_{ bounding_sphere.transformed(transformation_matrix) }
    , smesh_{ collision_mesh }
{}

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
    const BoundingSphere<double, 3>& bounding_sphere,
    const std::shared_ptr<CollisionMesh<double>>& collision_mesh,
    ScenePos max_min_cos_ridge)
    : max_min_cos_ridge_{ max_min_cos_ridge }
    , transformation_matrix_{ transformation_matrix }
    , transformed_bounding_sphere_{ bounding_sphere.transformed(transformation_matrix) }
    , dmesh_{ collision_mesh }
{}

LazyTransformedMesh::~LazyTransformedMesh() = default;

bool LazyTransformedMesh::intersects(const BoundingSphere<ScenePos, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool LazyTransformedMesh::intersects(const PlaneNd<ScenePos, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<ScenePos, 4>>& LazyTransformedMesh::get_quads_sphere() const {
    if (!quads_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!quads_calculated_) {
            transformed_quads_.reserve(
                (smesh_ == nullptr ? 0 : smesh_->quads.size()) +
                (dmesh_ == nullptr ? 0 : dmesh_->quads.size()));
            if (smesh_ != nullptr) {
                for (const auto& q : smesh_->quads) {
                    transformed_quads_.push_back(q.transformed(transformation_matrix_));
                }
            }
            if (dmesh_ != nullptr) {
                for (const auto& q : dmesh_->quads) {
                    transformed_quads_.push_back(q.transformed(transformation_matrix_));
                }
            }
            quads_calculated_ = true;
        }
    }
    return transformed_quads_;
}


const std::vector<CollisionPolygonSphere<ScenePos, 3>>& LazyTransformedMesh::get_triangles_sphere() const {
    if (!triangles_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!triangles_calculated_) {
            transformed_triangles_.reserve(
                (smesh_ == nullptr ? 0 : smesh_->triangles.size()) +
                (dmesh_ == nullptr ? 0 : dmesh_->triangles.size()));
            if (smesh_ != nullptr) {
                for (const auto& t : smesh_->triangles) {
                    transformed_triangles_.push_back(t.transformed(transformation_matrix_));
                }
            }
            if (dmesh_ != nullptr) {
                for (const auto& t : dmesh_->triangles) {
                    transformed_triangles_.push_back(t.transformed(transformation_matrix_));
                }
            }
            triangles_calculated_ = true;
        }
    }
    return transformed_triangles_;
}

const std::vector<CollisionLineSphere<ScenePos>>& LazyTransformedMesh::get_edges_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    lerr() << "Skipping mesh without triangles";
    //}
    if (std::isnan(max_min_cos_ridge_) && !edges_calculated_) {
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

const std::vector<CollisionRidgeSphere>& LazyTransformedMesh::get_ridges_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    lerr() << "Skipping mesh without triangles";
    //}
    if (!std::isnan(max_min_cos_ridge_) && !ridges_calculated_) {
        get_quads_sphere();
        get_triangles_sphere();
        std::scoped_lock lock{mutex_};
        if (!ridges_calculated_) {
            CollisionRidges ridges;
            for (const auto& q3 : transformed_quads_) {
                ridges.insert(q3.corners, q3.polygon.plane().normal, q3.vertex_normals, max_min_cos_ridge_, q3.physics_material);
            }
            for (const auto& t3 : transformed_triangles_) {
                ridges.insert(t3.corners, t3.polygon.plane().normal, t3.vertex_normals, max_min_cos_ridge_, t3.physics_material);
            }
            transformed_ridges_.reserve(ridges.size());
            for (const auto& e : ridges) {
                if (e.collision_ridge_sphere.is_touchable(SingleFaceBehavior::UNTOUCHABLE)) {
                    transformed_ridges_.emplace_back(e.collision_ridge_sphere).finalize();
                }
            }
            ridges_calculated_ = true;
        }
    }
    return transformed_ridges_;
}

const std::vector<CollisionLineSphere<ScenePos>>& LazyTransformedMesh::get_lines_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    lerr() << "Skipping mesh without triangles";
    //}
    if (!lines_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!lines_calculated_) {
            transformed_lines_.reserve(
                (smesh_ == nullptr ? 0 : smesh_->lines.size()) +
                (dmesh_ == nullptr ? 0 : dmesh_->lines.size()));
            if (smesh_ != nullptr) {
                for (const auto& l2 : smesh_->lines) {
                    transformed_lines_.push_back(l2.transformed(transformation_matrix_));
                }
            }
            if (dmesh_ != nullptr) {
                for (const auto& l2 : dmesh_->lines) {
                    transformed_lines_.push_back(l2.transformed(transformation_matrix_));
                }
            }
            lines_calculated_ = true;
        }
    }
    return transformed_lines_;
}

BoundingSphere<ScenePos, 3> LazyTransformedMesh::bounding_sphere() const {
    return transformed_bounding_sphere_;
}

AxisAlignedBoundingBox<ScenePos, 3> LazyTransformedMesh::aabb() const {
    return AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(
        transformed_bounding_sphere_.center(),
        transformed_bounding_sphere_.radius());
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

void LazyTransformedMesh::print_info() const {
    lerr() << "LazyTransformedMesh";
    lerr() << transformed_bounding_sphere_.center();
    lerr() << transformed_bounding_sphere_.radius();
}

std::string LazyTransformedMesh::name() const {
    return
        (smesh_ == nullptr ? "" : smesh_->name) + "_" + 
        (dmesh_ == nullptr ? "" : dmesh_->name);
}
