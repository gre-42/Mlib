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
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionLineSphere;
struct CollisionRidgeSphere;

class StaticTransformedMesh: public IIntersectableMesh {
    StaticTransformedMesh(const StaticTransformedMesh&) = delete;
    StaticTransformedMesh& operator = (const StaticTransformedMesh&) = delete;
public:
    StaticTransformedMesh(
        std::string name,
        const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb,
        const BoundingSphere<CompressedScenePos, 3>& bounding_sphere,
        std::vector<CollisionPolygonSphere<4>>&& quads,
        std::vector<CollisionPolygonSphere<3>>&& triangles,
        std::vector<CollisionLineSphere>&& lines,
        std::vector<CollisionLineSphere>&& edges,
        std::vector<CollisionRidgeSphere>&& ridges,
        std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables);
    ~StaticTransformedMesh();
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
private:
    std::string name_;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb_;
    BoundingSphere<CompressedScenePos, 3> bounding_sphere_;
    std::vector<CollisionPolygonSphere<4>> quads_;
    std::vector<CollisionPolygonSphere<3>> triangles_;
    std::vector<CollisionLineSphere> lines_;
    std::vector<CollisionLineSphere> edges_;
    std::vector<CollisionRidgeSphere> ridges_;
    std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables_;
};

}
