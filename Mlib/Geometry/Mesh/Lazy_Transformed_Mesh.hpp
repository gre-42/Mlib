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

class CollisionMesh;
enum class PhysicsMaterial: uint32_t;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionLineSphere;
template <class T>
struct TypedMesh;

class LazyTransformedMesh: public IIntersectableMesh {
    LazyTransformedMesh(const LazyTransformedMesh&) = delete;
    LazyTransformedMesh& operator = (const LazyTransformedMesh&) = delete;
public:
    LazyTransformedMesh(
        const TransformationMatrix<SceneDir, ScenePos, 3>& transformation_matrix,
        const BoundingSphere<CompressedScenePos, 3>& bounding_sphere,
        const std::shared_ptr<CollisionMesh>& collision_mesh,
        ScenePos max_min_cos_ridge);
    ~LazyTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<CompressedScenePos, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<ScenePos, 3>& plane) const override;
    virtual const std::vector<CollisionPolygonSphere<4>>& get_quads_sphere() const override;
    virtual const std::vector<CollisionPolygonSphere<3>>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere>& get_lines_sphere() const override;
    virtual const std::vector<CollisionLineSphere>& get_edges_sphere() const override;
    virtual const std::vector<CollisionRidgeSphere>& get_ridges_sphere() const override;
    virtual const std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>& get_intersectables() const override;
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const override;
    void print_info() const;
private:
    ScenePos max_min_cos_ridge_;
    const TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
    BoundingSphere<CompressedScenePos, 3> transformed_bounding_sphere_;
    std::shared_ptr<CollisionMesh> mesh_;
    mutable std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables_;
    mutable std::vector<CollisionPolygonSphere<4>> transformed_quads_;
    mutable std::vector<CollisionPolygonSphere<3>> transformed_triangles_;
    mutable std::vector<CollisionLineSphere> transformed_lines_;
    mutable std::vector<CollisionLineSphere> transformed_edges_;
    mutable std::vector<CollisionRidgeSphere> transformed_ridges_;
    mutable std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> transformed_intersectables_;
    mutable AtomicMutex mutex_;
    mutable std::atomic_bool quads_calculated_ = false;
    mutable std::atomic_bool triangles_calculated_ = false;
    mutable std::atomic_bool lines_calculated_ = false;
    mutable std::atomic_bool edges_calculated_ = false;
    mutable std::atomic_bool ridges_calculated_ = false;
    mutable std::atomic_bool intersectables_calculated_ = false;
};

}
