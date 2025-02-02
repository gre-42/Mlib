#pragma once
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class T>
struct TypedMesh;
class IIntersectable;

class CollisionMesh {
    CollisionMesh(const CollisionMesh&) = delete;
    CollisionMesh& operator = (const CollisionMesh&) = delete;
public:
    template <class TData>
    explicit CollisionMesh(const ColoredVertexArray<TData>& mesh);
    CollisionMesh(
        std::string name,
        TypedMesh<std::shared_ptr<IIntersectable>> intersectable);
    ~CollisionMesh();
    std::string name;
    std::vector<CollisionPolygonSphere<CompressedScenePos, 4>> quads;
    std::vector<CollisionPolygonSphere<CompressedScenePos, 3>> triangles;
    std::vector<CollisionLineSphere<CompressedScenePos>> lines;
    TypedMesh<std::shared_ptr<IIntersectable>> intersectable;
};

}
