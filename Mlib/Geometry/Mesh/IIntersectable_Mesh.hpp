#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

template <class TPos, size_t tndim>
class BoundingSphere;
template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition>
struct CollisionRidgeSphere;
template <class TPosition>
struct CollisionLineSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
class CollisionVertices;
template <class TData, size_t... tshape>
class OrderableFixedArray;
class IIntersectable;
template <class T>
struct TypedMesh;

class IIntersectableMesh {
public:
    IIntersectableMesh();
    virtual ~IIntersectableMesh();
    virtual std::string name() const = 0;
    bool intersects(const IIntersectableMesh& other) const;
    virtual bool intersects(const BoundingSphere<CompressedScenePos, 3>& sphere) const = 0;
    virtual bool intersects(const PlaneNd<SceneDir, CompressedScenePos, 3>& plane) const = 0;
    template <size_t tnvertices>
    inline const std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& get_polygons_sphere() const {
        if constexpr (tnvertices == 4) {
            return get_quads_sphere();
        } else if constexpr (tnvertices == 3) {
            return get_triangles_sphere();
        } else {
            // From: https://stackoverflow.com/questions/38304847/constexpr-if-and-static-assert
            static_assert(tnvertices == 4, "Unknown vertex-count");
        }
    }
    virtual const std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& get_quads_sphere() const = 0;
    virtual const std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& get_triangles_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere<CompressedScenePos>>& get_lines_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere<CompressedScenePos>>& get_edges_sphere() const = 0;
    virtual const std::vector<CollisionRidgeSphere<CompressedScenePos>>& get_ridges_sphere() const = 0;
    virtual const std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>& get_intersectables() const = 0;
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& get_vertices() const;
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const = 0;
private:
    mutable std::unique_ptr<CollisionVertices> collision_vertices_;
    mutable SafeAtomicSharedMutex mutex_;
};

}
