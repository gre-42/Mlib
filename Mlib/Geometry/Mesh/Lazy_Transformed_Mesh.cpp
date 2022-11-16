#include "Lazy_Transformed_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<double, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<double>>& dmesh)
: transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{bounding_sphere.transformed(transformation_matrix)},
  dmesh_{ dmesh }
{}

LazyTransformedMesh::LazyTransformedMesh(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<float, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<float>>& smesh)
: transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{bounding_sphere.transformed(transformation_matrix)},
  smesh_{ smesh }
{}

LazyTransformedMesh::LazyTransformedMesh(
    const BoundingSphere<double, 3>& transformed_bounding_sphere,
    const std::vector<CollisionTriangleSphere>& transformed_triangles)
: transformation_matrix_{ fixed_nans<double, 4, 4>() },
  transformed_bounding_sphere_{ transformed_bounding_sphere },
  transformed_triangles_{ transformed_triangles },
  triangles_calculated_{ true }
{}

LazyTransformedMesh::~LazyTransformedMesh()
{}

bool LazyTransformedMesh::intersects(const BoundingSphere<double, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool LazyTransformedMesh::intersects(const PlaneNd<double, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionTriangleSphere>& LazyTransformedMesh::get_triangles_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!triangles_calculated_) {
        std::lock_guard<std::mutex> lock{mutex_};
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

const std::vector<CollisionLineSphere>& LazyTransformedMesh::get_lines_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!lines_calculated_) {
        std::lock_guard<std::mutex> lock{mutex_};
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
