#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class PlaneNd;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionRidgeSphere;
struct CollisionLineSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

class IIntersectableMesh {
public:
    virtual ~IIntersectableMesh() = default;
    virtual std::string name() const = 0;
    bool intersects(const IIntersectableMesh& other) const;
    virtual bool intersects(const BoundingSphere<double, 3>& sphere) const = 0;
    virtual bool intersects(const PlaneNd<double, 3>& plane) const = 0;
    template <size_t tnvertices>
    inline const std::vector<CollisionPolygonSphere<tnvertices>>& get_polygons_sphere() const {
        if constexpr (tnvertices == 4) {
            return get_quads_sphere();
        } else if constexpr (tnvertices == 3) {
            return get_triangles_sphere();
        } else {
            // From: https://stackoverflow.com/questions/38304847/constexpr-if-and-static-assert
            static_assert(tnvertices == 4, "Unknown vertex-count");
        }
    }
    virtual const std::vector<CollisionPolygonSphere<4>>& get_quads_sphere() const = 0;
    virtual const std::vector<CollisionPolygonSphere<3>>& get_triangles_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere>& get_lines_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere>& get_edges_sphere() const = 0;
    virtual const std::vector<CollisionRidgeSphere>& get_ridges_sphere() const = 0;
    virtual BoundingSphere<double, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const = 0;
};

}
