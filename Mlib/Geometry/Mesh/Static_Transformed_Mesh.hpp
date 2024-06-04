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
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
struct CollisionLineSphere;
struct CollisionRidgeSphere;

class StaticTransformedMesh: public IIntersectableMesh {
    StaticTransformedMesh(const StaticTransformedMesh&) = delete;
    StaticTransformedMesh& operator = (const StaticTransformedMesh&) = delete;
public:
    StaticTransformedMesh(
        std::string name,
        const AxisAlignedBoundingBox<double, 3>& aabb,
        const BoundingSphere<double, 3>& bounding_sphere,
        std::vector<CollisionPolygonSphere<double, 4>>&& quads,
        std::vector<CollisionPolygonSphere<double, 3>>&& triangles,
        std::vector<CollisionLineSphere<double>>&& lines,
        std::vector<CollisionLineSphere<double>>&& edges,
        std::vector<CollisionRidgeSphere>&& ridges);
    ~StaticTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<double, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<double, 3>& plane) const override;
    virtual const std::vector<CollisionPolygonSphere<double, 4>>& get_quads_sphere() const override;
    virtual const std::vector<CollisionPolygonSphere<double, 3>>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere<double>>& get_lines_sphere() const override;
    virtual const std::vector<CollisionLineSphere<double>>& get_edges_sphere() const override;
    virtual const std::vector<CollisionRidgeSphere>& get_ridges_sphere() const override;
    virtual BoundingSphere<double, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const override;
private:
    std::string name_;
    AxisAlignedBoundingBox<double, 3> aabb_;
    BoundingSphere<double, 3> bounding_sphere_;
    std::vector<CollisionPolygonSphere<double, 4>> quads_;
    std::vector<CollisionPolygonSphere<double, 3>> triangles_;
    std::vector<CollisionLineSphere<double>> lines_;
    std::vector<CollisionLineSphere<double>> edges_;
    std::vector<CollisionRidgeSphere> ridges_;
};

}
