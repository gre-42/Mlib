#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <vector>

namespace Mlib {

template <class TPos, size_t tndim>
class BoundingSphere;
template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition>
struct CollisionLineSphere;
template <class TPosition>
struct CollisionRidgeSphere;

class StaticTransformedMesh: public IIntersectableMesh {
    StaticTransformedMesh(const StaticTransformedMesh&) = delete;
    StaticTransformedMesh& operator = (const StaticTransformedMesh&) = delete;
public:
    StaticTransformedMesh(
        std::string name,
        const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb,
        const BoundingSphere<CompressedScenePos, 3>& bounding_sphere,
        std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>&& quads,
        std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>&& triangles,
        std::vector<CollisionLineSphere<CompressedScenePos>>&& lines,
        std::vector<CollisionLineSphere<CompressedScenePos>>&& edges,
        std::vector<CollisionRidgeSphere<CompressedScenePos>>&& ridges,
        std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables);
    ~StaticTransformedMesh();
    virtual std::string name() const override;
    virtual bool intersects(const BoundingSphere<CompressedScenePos, 3>& sphere) const override;
    virtual bool intersects(const PlaneNd<SceneDir, CompressedScenePos, 3>& plane) const override;
    virtual const std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>& get_quads_sphere() const override;
    virtual const std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>& get_triangles_sphere() const override;
    virtual const std::vector<CollisionLineSphere<CompressedScenePos>>& get_lines_sphere() const override;
    virtual const std::vector<CollisionLineSphere<CompressedScenePos>>& get_edges_sphere() const override;
    virtual const std::vector<CollisionRidgeSphere<CompressedScenePos>>& get_ridges_sphere() const override;
    virtual const std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>& get_intersectables() const override;
    virtual BoundingSphere<CompressedScenePos, 3> bounding_sphere() const override;
    virtual AxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const override;
private:
    std::string name_;
    AxisAlignedBoundingBox<CompressedScenePos, 3> aabb_;
    BoundingSphere<CompressedScenePos, 3> bounding_sphere_;
    std::vector<CollisionPolygonSphere<CompressedScenePos, 4>> quads_;
    std::vector<CollisionPolygonSphere<CompressedScenePos, 3>> triangles_;
    std::vector<CollisionLineSphere<CompressedScenePos>> lines_;
    std::vector<CollisionLineSphere<CompressedScenePos>> edges_;
    std::vector<CollisionRidgeSphere<CompressedScenePos>> ridges_;
    std::vector<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables_;
};

}
