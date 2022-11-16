#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class PlaneNd;
struct CollisionTriangleSphere;
struct CollisionLineSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

class IntersectableMesh {
public:
    virtual ~IntersectableMesh() = default;
    virtual std::string name() const = 0;
    bool intersects(const IntersectableMesh& other) const;
    virtual bool intersects(const BoundingSphere<double, 3>& sphere) const = 0;
    virtual bool intersects(const PlaneNd<double, 3>& plane) const = 0;
    virtual const std::vector<CollisionTriangleSphere>& get_triangles_sphere() const = 0;
    virtual const std::vector<CollisionLineSphere>& get_lines_sphere() const = 0;
    virtual BoundingSphere<double, 3> bounding_sphere() const = 0;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const = 0;
};

}
