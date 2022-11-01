#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <atomic>
#include <mutex>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
enum class PhysicsMaterial;
struct CollisionTriangleSphere;
struct CollisionLineSphere;

class TransformedMesh {
public:
    TransformedMesh(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BoundingSphere<float, 3>& bounding_sphere,
        const std::shared_ptr<ColoredVertexArray<float>>& smesh);
    TransformedMesh(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BoundingSphere<double, 3>& bounding_sphere,
        const std::shared_ptr<ColoredVertexArray<double>>& dmesh);
    TransformedMesh(
        const BoundingSphere<double, 3>& transformed_bounding_sphere,
        const std::vector<CollisionTriangleSphere>& transformed_triangles);
    ~TransformedMesh();
    bool intersects(const TransformedMesh& other) const;
    bool intersects(const BoundingSphere<double, 3>& sphere) const;
    bool intersects(const PlaneNd<double, 3>& plane) const;
    const std::vector<CollisionTriangleSphere>& get_triangles_sphere() const;
    const std::vector<CollisionLineSphere>& get_lines_sphere() const;
    void print_info() const;
    const BoundingSphere<double, 3>& transformed_bounding_sphere() const;
private:
    const TransformationMatrix<float, double, 3> transformation_matrix_;
    BoundingSphere<double, 3> transformed_bounding_sphere_;
    std::shared_ptr<ColoredVertexArray<float>> smesh_;
    std::shared_ptr<ColoredVertexArray<double>> dmesh_;
    mutable std::vector<CollisionTriangleSphere> transformed_triangles_;
    mutable std::vector<CollisionLineSphere> transformed_lines_;
    mutable std::mutex mutex_;
    mutable std::atomic_bool triangles_calculated_ = false;
    mutable std::atomic_bool lines_calculated_ = false;
};

}
