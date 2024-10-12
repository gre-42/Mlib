#pragma once
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
enum class NormalVectorErrorBehavior;

template <class TData>
class CollisionMesh {
    CollisionMesh(const CollisionMesh&) = delete;
    CollisionMesh& operator = (const CollisionMesh&) = delete;
public:
    explicit CollisionMesh(
        const ColoredVertexArray<TData>& mesh,
        NormalVectorErrorBehavior zero_normal_behavior);
    ~CollisionMesh();
    std::string name;
    std::vector<CollisionPolygonSphere<TData, 4>> quads;
    std::vector<CollisionPolygonSphere<TData, 3>> triangles;
    std::vector<CollisionLineSphere<TData>> lines;
};

}
