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
template <class TData>
struct CollisionRidgeSphere;

class StaticTransformedMesh: public IIntersectableMesh {
    StaticTransformedMesh(const StaticTransformedMesh&) = delete;
    StaticTransformedMesh& operator = (const StaticTransformedMesh&) = delete;
public:
    StaticTransformedMesh(
        std::string name,
        const AxisAlignedBoundingBox<ScenePos, 3>& aabb,
        const BoundingSphere<ScenePos, 3>& bounding_sphere,
        std::vector<CollisionPolygonSphere<ScenePos, 4>>&& quads,
        std::vector<CollisionPolygonSphere<ScenePos, 3>>&& triangles,
        std::vector<CollisionLineSphere<ScenePos>>&& lines,
        std::vector<CollisionLineSphere<ScenePos>>&& edges,
        std::vector<CollisionRidgeSphere<ScenePos>>&& ridges,
        std::vector<TypedMesh<std::shared_ptr<IIntersectable<ScenePos>>>>&& intersectables);
    ~StaticTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<ScenePos, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<ScenePos, 3>& plane) const override;
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 4>>& get_quads_sphere() const override;
    virtual const std::vector<CollisionPolygonSphere<ScenePos, 3>>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_lines_sphere() const override;
    virtual const std::vector<CollisionLineSphere<ScenePos>>& get_edges_sphere() const override;
    virtual const std::vector<CollisionRidgeSphere<ScenePos>>& get_ridges_sphere() const override;
    virtual const std::vector<TypedMesh<std::shared_ptr<IIntersectable<ScenePos>>>>& get_intersectables() const override;
    virtual BoundingSphere<ScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<ScenePos, 3> aabb() const override;
private:
    std::string name_;
    AxisAlignedBoundingBox<ScenePos, 3> aabb_;
    BoundingSphere<ScenePos, 3> bounding_sphere_;
    std::vector<CollisionPolygonSphere<ScenePos, 4>> quads_;
    std::vector<CollisionPolygonSphere<ScenePos, 3>> triangles_;
    std::vector<CollisionLineSphere<ScenePos>> lines_;
    std::vector<CollisionLineSphere<ScenePos>> edges_;
    std::vector<CollisionRidgeSphere<ScenePos>> ridges_;
    std::vector<TypedMesh<std::shared_ptr<IIntersectable<ScenePos>>>> intersectables_;
};

}
