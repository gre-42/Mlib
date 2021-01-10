#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <atomic>
#include <mutex>
#include <vector>

namespace Mlib {

class ColoredVertexArray;

class TransformedMesh {
public:
    TransformedMesh(
        const TransformationMatrix<float>& transformation_matrix,
        const BoundingSphere<float, 3>& bounding_sphere,
        const std::shared_ptr<ColoredVertexArray>& mesh);
    TransformedMesh(
        const BoundingSphere<float, 3>& transformed_bounding_sphere,
        const std::vector<CollisionTriangleSphere>& transformed_triangles);
    bool intersects(const TransformedMesh& other) const;
    bool intersects(const BoundingSphere<float, 3>& sphere) const;
    bool intersects(const PlaneNd<float, 3>& plane) const;
    const std::vector<CollisionTriangleSphere>& get_triangles_sphere() const;
    const std::vector<FixedArray<FixedArray<float, 3>, 2>>& get_lines() const;
    void print_info() const;
    const BoundingSphere<float, 3>& transformed_bounding_sphere() const;
private:
    const TransformationMatrix<float> transformation_matrix_;
    BoundingSphere<float, 3> transformed_bounding_sphere_;
    std::shared_ptr<ColoredVertexArray> mesh_;
    mutable std::vector<CollisionTriangleSphere> transformed_triangles_;
    mutable std::vector<FixedArray<FixedArray<float, 3>, 2>> transformed_lines_;
    mutable std::mutex mutex_;
    mutable std::atomic_bool triangles_calculated_ = false;
    mutable std::atomic_bool lines_calculated_ = false;
};

}
