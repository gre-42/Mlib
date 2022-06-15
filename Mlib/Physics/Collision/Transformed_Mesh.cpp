#include "Transformed_Mesh.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif
TransformedMesh::TransformedMesh(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<double, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<double>>& dmesh)
: transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{
    transformation_matrix.transform(bounding_sphere.center()),
    bounding_sphere.radius() * transformation_matrix_.get_scale()},
  dmesh_{ dmesh }
{}

TransformedMesh::TransformedMesh(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BoundingSphere<float, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray<float>>& smesh)
: transformation_matrix_{ transformation_matrix },
  transformed_bounding_sphere_{
    transformation_matrix.transform(bounding_sphere.center().casted<double>()),
    bounding_sphere.radius() * transformation_matrix_.get_scale()},
  smesh_{ smesh }
{}

TransformedMesh::TransformedMesh(
    const BoundingSphere<double, 3>& transformed_bounding_sphere,
    const std::vector<CollisionTriangleSphere>& transformed_triangles)
: transformation_matrix_{ fixed_nans<double, 4, 4>() },
  transformed_bounding_sphere_{ transformed_bounding_sphere },
  transformed_triangles_{ transformed_triangles },
  triangles_calculated_{ true }
{}

bool TransformedMesh::intersects(const TransformedMesh& other) const {
    return transformed_bounding_sphere_.intersects(other.transformed_bounding_sphere_);
}

bool TransformedMesh::intersects(const BoundingSphere<double, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool TransformedMesh::intersects(const PlaneNd<double, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionTriangleSphere>& TransformedMesh::get_triangles_sphere() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!triangles_calculated_) {
        std::lock_guard<std::mutex> lock{mutex_};
        if (!triangles_calculated_) {
            if (smesh_ != nullptr) {
                transformed_triangles_ = smesh_->transformed_triangles_sphere(transformation_matrix_);
            }
            if (dmesh_ != nullptr) {
                auto dtt = dmesh_->transformed_triangles_sphere(transformation_matrix_);
                transformed_triangles_.insert(transformed_triangles_.end(), dtt.begin(), dtt.end());
            }
            triangles_calculated_ = true;
        }
    }
    return transformed_triangles_;
}

const std::vector<FixedArray<FixedArray<double, 3>, 2>>& TransformedMesh::get_lines() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!lines_calculated_) {
        std::lock_guard<std::mutex> lock{mutex_};
        if (!lines_calculated_) {
            if (smesh_ != nullptr) {
                transformed_lines_ = smesh_->transformed_lines<double>(transformation_matrix_);
            }
            if (dmesh_ != nullptr) {
                auto dtl = dmesh_->transformed_lines<double>(transformation_matrix_);
                transformed_lines_.insert(transformed_lines_.end(), dtl.begin(), dtl.end());
            }
            lines_calculated_ = true;
        }
    }
    return transformed_lines_;
}

const BoundingSphere<double, 3>& TransformedMesh::transformed_bounding_sphere() const {
    return transformed_bounding_sphere_;
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

void TransformedMesh::print_info() const {
    std::cerr << "TransformedMesh" << std::endl;
    std::cerr << transformed_bounding_sphere_.center() << std::endl;
    std::cerr << transformed_bounding_sphere_.radius() << std::endl;
}
