#include "Lazy_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/Collision_Edges.hpp>
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
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<double, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<double>>& dmesh,
    double max_min_cos_ridge)
: max_min_cos_ridge_{max_min_cos_ridge},
  transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{bounding_sphere.transformed(transformation_matrix)},
  dmesh_{ dmesh }
{}

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<float, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<float>>& smesh,
    double max_min_cos_ridge)
: max_min_cos_ridge_{ max_min_cos_ridge },
  transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{ bounding_sphere.transformed(transformation_matrix) },
  smesh_{ smesh }
{}

LazyTransformedMesh::~LazyTransformedMesh()
{}

bool LazyTransformedMesh::intersects(const BoundingSphere<double, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool LazyTransformedMesh::intersects(const PlaneNd<double, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionPolygonSphere<4>>& LazyTransformedMesh::get_quads_sphere() const {
    if (!quads_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!quads_calculated_) {
            transformed_quads_.reserve(
                (smesh_ == nullptr ? 0 : smesh_->quads.size()) +
                (dmesh_ == nullptr ? 0 : dmesh_->quads.size()));
            if (smesh_ != nullptr) {
                smesh_->transformed_quads_sphere(transformed_quads_, transformation_matrix_);
            }
            if (dmesh_ != nullptr) {
                dmesh_->transformed_quads_sphere(transformed_quads_, transformation_matrix_);
            }
            quads_calculated_ = true;
        }
    }
    return transformed_quads_;
}


const std::vector<CollisionPolygonSphere<3>>& LazyTransformedMesh::get_triangles_sphere() const {
    if (!triangles_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!triangles_calculated_) {
            transformed_triangles_.reserve(
                (smesh_ == nullptr ? 0 : smesh_->triangles.size()) +
                (dmesh_ == nullptr ? 0 : dmesh_->triangles.size()));
            if (smesh_ != nullptr) {
                smesh_->transformed_triangles_sphere(transformed_triangles_, transformation_matrix_);
            }
            if (dmesh_ != nullptr) {
                dmesh_->transformed_triangles_sphere(transformed_triangles_, transformation_matrix_);
            }
            triangles_calculated_ = true;
        }
    }
    return transformed_triangles_;
}

const std::vector<CollisionLineSphere>& LazyTransformedMesh::get_edges_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (std::isnan(max_min_cos_ridge_) && !edges_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!edges_calculated_) {
            CollisionEdges edges;
            if (smesh_ != nullptr) {
                for (const auto& t : smesh_->triangles) {
                    Triangle3D t3{t, transformation_matrix_};
                    edges.insert(t3.vertices(), smesh_->physics_material);
                }
                for (const auto& t : smesh_->quads) {
                    Quad3D q3{t, transformation_matrix_};
                    edges.insert(q3.vertices(), smesh_->physics_material);
                }
            }
            if (dmesh_ != nullptr) {
                for (const auto& t : dmesh_->triangles) {
                    Triangle3D t3{t, transformation_matrix_};
                    edges.insert(t3.vertices(), smesh_->physics_material);
                }
                for (const auto& t : dmesh_->quads) {
                    Quad3D q3{t, transformation_matrix_};
                    edges.insert(q3.vertices(), smesh_->physics_material);
                }
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
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!std::isnan(max_min_cos_ridge_) && !ridges_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!ridges_calculated_) {
            CollisionRidges ridges;
            if (smesh_ != nullptr) {
                for (const auto& t : smesh_->triangles) {
                    Triangle3D t3{t, transformation_matrix_};
                    ridges.insert(t3.vertices(), t3.polygon().plane().normal, max_min_cos_ridge_, smesh_->physics_material);
                }
                for (const auto& t : smesh_->quads) {
                    Quad3D q3{t, transformation_matrix_};
                    ridges.insert(q3.vertices(), q3.polygon().plane().normal, max_min_cos_ridge_, smesh_->physics_material);
                }
            }
            if (dmesh_ != nullptr) {
                for (const auto& t : dmesh_->triangles) {
                    Triangle3D t3{t, transformation_matrix_};
                    ridges.insert(t3.vertices(), t3.polygon().plane().normal, max_min_cos_ridge_, smesh_->physics_material);
                }
                for (const auto& t : dmesh_->quads) {
                    Quad3D q3{t, transformation_matrix_};
                    ridges.insert(q3.vertices(), q3.polygon().plane().normal, max_min_cos_ridge_, smesh_->physics_material);
                }
            }
            transformed_ridges_.reserve(ridges.size());
            for (const auto& e : ridges) {
                if (e.collision_ridge_sphere.is_touchable(SingleFaceBehavior::UNTOUCHEABLE)) {
                    transformed_ridges_.emplace_back(e.collision_ridge_sphere).finalize();
                }
            }
            ridges_calculated_ = true;
        }
    }
    return transformed_ridges_;
}

const std::vector<CollisionLineSphere>& LazyTransformedMesh::get_lines_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!lines_calculated_) {
        std::scoped_lock lock{mutex_};
        if (!lines_calculated_) {
            if (smesh_ != nullptr) {
                transformed_lines_ = smesh_->transformed_lines_sphere(transformation_matrix_);
            }
            if (dmesh_ != nullptr) {
                auto dtl = dmesh_->transformed_lines_sphere(transformation_matrix_);
                transformed_lines_.insert(transformed_lines_.end(), dtl.begin(), dtl.end());
            }
            lines_calculated_ = true;
        }
    }
    return transformed_lines_;
}

BoundingSphere<double, 3> LazyTransformedMesh::bounding_sphere() const {
    return transformed_bounding_sphere_;
}

AxisAlignedBoundingBox<double, 3> LazyTransformedMesh::aabb() const {
    return AxisAlignedBoundingBox<double, 3>{
        transformed_bounding_sphere_.center(),
        transformed_bounding_sphere_.radius()};
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

void LazyTransformedMesh::print_info() const {
    std::cerr << "LazyTransformedMesh" << std::endl;
    std::cerr << transformed_bounding_sphere_.center() << std::endl;
    std::cerr << transformed_bounding_sphere_.radius() << std::endl;
}

std::string LazyTransformedMesh::name() const {
    return
        (smesh_ == nullptr ? "" : smesh_->name) + "_" + 
        (dmesh_ == nullptr ? "" : dmesh_->name);
}
