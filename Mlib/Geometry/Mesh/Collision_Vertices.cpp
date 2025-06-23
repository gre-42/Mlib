#include "Collision_Vertices.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

void CollisionVertices::insert(const FixedArray<CompressedScenePos, 4, 3>& quad) {
    insert(quad[0]);
    insert(quad[1]);
    insert(quad[2]);
    insert(quad[3]);
}

void CollisionVertices::insert(const FixedArray<CompressedScenePos, 3, 3>& tri) {
    insert(tri[0]);
    insert(tri[1]);
    insert(tri[2]);
}

void CollisionVertices::insert(const FixedArray<CompressedScenePos, 2, 3>& line) {
    insert(line[0]);
    insert(line[1]);
}

void CollisionVertices::insert(const FixedArray<CompressedScenePos, 3>& vertex) {
    vertices_.insert(OrderableFixedArray(vertex));
}

CollisionVertices::const_iterator CollisionVertices::begin() const {
    return vertices_.begin();
}

CollisionVertices::const_iterator CollisionVertices::end() const {
    return vertices_.end();
}
