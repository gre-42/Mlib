#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class PlaneNd;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionRidgeSphere;
template <class TData>
struct CollisionLineSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
class CollisionVertices;
template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;

class IIntersectableMesh {
public:
    IIntersectableMesh();
    virtual ~IIntersectableMesh();
    virtual std::string name() const = 0;
    bool intersects(const IIntersectableMesh& other) const;
    virtual bool intersects(const BoundingSphere<ScenePos, 3>& sphere) const = 0;
    virtual bool intersects(const PlaneNd<ScenePos, 3>& plane) const = 0;
    template <size_t tnvertices>
    inline const std::vector<CollisionPolygonSphere<ScenePos, tnvertices>>& get_polygons_sphere() const {
        if constexpr (tnvertices == 4) {
            return get_quads_sphere();
        } else if constexpr (tnvertices == 3) {
            return get_triangles_sphere();
        } else {
            // From: https://stackoverflow.com/questions/38304847/constexpr-if-and-static-assert
            static_assert(tnvertices == 4, "Unknown vertex-count");
        }
    }
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 4>>& get_quads_sphere() const = 0;
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 3>>& get_triangles_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_lines_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_edges_sphere() const = 0;
    virtual const std::vector<CollisionRidgeSphere>& get_ridges_sphere() const = 0;
    const std::set<OrderableFixedArray<ScenePos, 3>>& get_vertices() const;
    virtual BoundingSphere<ScenePos, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<ScenePos, 3> aabb() const = 0;
private:
    mutable std::unique_ptr<CollisionVertices> collision_vertices_;
    mutable SafeAtomicSharedMutex mutex_;
};

}
