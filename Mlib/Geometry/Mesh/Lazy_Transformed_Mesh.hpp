#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <atomic>
#include <cstdint>
#include <vector>

namespace Mlib {

template <class TData>
class CollisionMesh;
enum class PhysicsMaterial: uint32_t;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
struct CollisionLineSphere;

class LazyTransformedMesh: public IIntersectableMesh {
    LazyTransformedMesh(const LazyTransformedMesh&) = delete;
    LazyTransformedMesh& operator = (const LazyTransformedMesh&) = delete;
public:
    LazyTransformedMesh(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
        const BoundingSphere<float, 3>& bounding_sphere,
        const std::shared_ptr<CollisionMesh<float>>& collision_mesh,
        ScenePos max_min_cos_ridge);
    LazyTransformedMesh(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
        const BoundingSphere<double, 3>& bounding_sphere,
        const std::shared_ptr<CollisionMesh<double>>& collision_mesh,
        ScenePos max_min_cos_ridge);
    ~LazyTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<ScenePos, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<ScenePos, 3>& plane) const override;
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 4>>& get_quads_sphere() const override;
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 3>>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_lines_sphere() const override;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_edges_sphere() const override;
    virtual const std::vector<CollisionRidgeSphere>& get_ridges_sphere() const override;
    virtual BoundingSphere<ScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<ScenePos, 3> aabb() const override;
    void print_info() const;
private:
    ScenePos max_min_cos_ridge_;
    const TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
    BoundingSphere<ScenePos, 3> transformed_bounding_sphere_;
    std::shared_ptr<CollisionMesh<float>> smesh_;
    std::shared_ptr<CollisionMesh<double>> dmesh_;
    mutable std::vector<CollisionPolygonSphere<ScenePos, 4>> transformed_quads_;
    mutable std::vector<CollisionPolygonSphere<ScenePos, 3>> transformed_triangles_;
    mutable std::vector<CollisionLineSphere<ScenePos>> transformed_lines_;
    mutable std::vector<CollisionLineSphere<ScenePos>> transformed_edges_;
    mutable std::vector<CollisionRidgeSphere> transformed_ridges_;
    mutable AtomicMutex mutex_;
    mutable std::atomic_bool quads_calculated_ = false;
    mutable std::atomic_bool triangles_calculated_ = false;
    mutable std::atomic_bool lines_calculated_ = false;
    mutable std::atomic_bool edges_calculated_ = false;
    mutable std::atomic_bool ridges_calculated_ = false;
};

}
