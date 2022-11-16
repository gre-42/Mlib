#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Intersectable_Mesh.hpp>
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

class LazyTransformedMesh: public IntersectableMesh {
    LazyTransformedMesh(const LazyTransformedMesh&) = delete;
    LazyTransformedMesh& operator = (const LazyTransformedMesh&) = delete;
public:
    LazyTransformedMesh(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BoundingSphere<float, 3>& bounding_sphere,
        const std::shared_ptr<ColoredVertexArray<float>>& smesh);
    LazyTransformedMesh(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BoundingSphere<double, 3>& bounding_sphere,
        const std::shared_ptr<ColoredVertexArray<double>>& dmesh);
    LazyTransformedMesh(
        const BoundingSphere<double, 3>& transformed_bounding_sphere,
        const std::vector<CollisionTriangleSphere>& transformed_triangles);
    ~LazyTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<double, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<double, 3>& plane) const override;
    virtual const std::vector<CollisionTriangleSphere>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere>& get_lines_sphere() const override;
    virtual BoundingSphere<double, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const override;
    void print_info() const;
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
