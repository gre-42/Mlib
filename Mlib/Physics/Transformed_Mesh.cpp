#include "Transformed_Mesh.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

#pragma GCC push_options
#pragma GCC optimize ("O3")
TransformedMesh::TransformedMesh(
    const FixedArray<float, 4, 4>& transformation_matrix,
    const BoundingSphere<float, 3>& bounding_sphere,
    const std::shared_ptr<ColoredVertexArray>& mesh)
: transformation_matrix_{transformation_matrix},
  transformed_bounding_sphere_{
    dehomogenized_3(dot1d(transformation_matrix, homogenized_4(bounding_sphere.center()))),
    bounding_sphere.radius() * std::sqrt(sum(squared(R3_from_4x4(transformation_matrix_))) / 3)},
  mesh_{mesh}
{}

TransformedMesh::TransformedMesh(
    const BoundingSphere<float, 3>& transformed_bounding_sphere,
    const std::vector<CollisionTriangle>& transformed_triangles)
: transformation_matrix_{fixed_nans<float, 4, 4>()},
  transformed_bounding_sphere_{transformed_bounding_sphere},
  transformed_triangles_{transformed_triangles},
  triangles_calculated_{true}
{}

bool TransformedMesh::intersects(const TransformedMesh& other) const {
    return transformed_bounding_sphere_.intersects(other.transformed_bounding_sphere_);
}

bool TransformedMesh::intersects(const BoundingSphere<float, 3>& sphere) const {
    return transformed_bounding_sphere_.intersects(sphere);
}

bool TransformedMesh::intersects(const PlaneNd<float, 3>& plane) const {
    return transformed_bounding_sphere_.intersects(plane);
}

const std::vector<CollisionTriangle>& TransformedMesh::get_triangles() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!triangles_calculated_) {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            if (!triangles_calculated_) {
                transformed_triangles_ = mesh_->transformed_triangles(transformation_matrix_);
            }
        }
        triangles_calculated_ = true;
    }
    return transformed_triangles_;
}

const std::vector<FixedArray<FixedArray<float, 3>, 2>>& TransformedMesh::get_lines() const {
    //if (msh.vertices->size() == 0) {
    //    std::cerr << "Skipping mesh without triangles" << std::endl;
    //}
    if (!lines_calculated_) {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            if (!lines_calculated_) {
                transformed_lines_ = mesh_->transformed_lines(transformation_matrix_);
            }
        }
        lines_calculated_ = true;
    }
    return transformed_lines_;
}

#pragma GCC pop_options

void TransformedMesh::print_info() const {
    std::cerr << "TransformedMesh" << std::endl;
    std::cerr << transformed_bounding_sphere_.center() << std::endl;
    std::cerr << transformed_bounding_sphere_.radius() << std::endl;
}
