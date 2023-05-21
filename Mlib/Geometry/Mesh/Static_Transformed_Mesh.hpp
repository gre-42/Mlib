#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <vector>

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class PlaneNd;
struct CollisionTriangleSphere;
struct CollisionLineSphere;

class StaticTransformedMesh: public IIntersectableMesh {
public:
    StaticTransformedMesh(
        const std::string& name,
        const AxisAlignedBoundingBox<double, 3>& aabb,
        const BoundingSphere<double, 3>& bounding_sphere,
        std::vector<CollisionTriangleSphere>&& triangles,
        std::vector<CollisionLineSphere>&& lines);
    ~StaticTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<double, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<double, 3>& plane) const override;
    virtual const std::vector<CollisionTriangleSphere>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere>& get_lines_sphere() const override;
    virtual BoundingSphere<double, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const override;
private:
    std::string name_;
    AxisAlignedBoundingBox<double, 3> aabb_;
    BoundingSphere<double, 3> bounding_sphere_;
    std::vector<CollisionTriangleSphere> triangles_;
    std::vector<CollisionLineSphere> lines_;
};

}
