#include "Collision_Edges.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>
    OrderableEdgeSphere::key() const
{
    if (OrderableFixedArray(collision_line_sphere.line[0]) > OrderableFixedArray(collision_line_sphere.line[1])) {
        return std::make_pair(
            OrderableFixedArray(collision_line_sphere.line[0]),
            OrderableFixedArray(collision_line_sphere.line[1]));
    } else {
        return std::make_pair(
            OrderableFixedArray(collision_line_sphere.line[1]),
            OrderableFixedArray(collision_line_sphere.line[0]));
    }
}

bool OrderableEdgeSphere::operator < (const OrderableEdgeSphere& other) const {
    return key() < other.key();
}

CollisionEdges::CollisionEdges() = default;

CollisionEdges::~CollisionEdges() = default;

template <size_t tnvertices>
void CollisionEdges::insert(
    const FixedArray<CompressedScenePos, tnvertices, 3>& tri,
    PhysicsMaterial physics_material)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        insert(tri[i], tri[(i + 1) % tnvertices], physics_material);
    }
}

void CollisionEdges::insert(
    const FixedArray<CompressedScenePos, 3>& a,
    const FixedArray<CompressedScenePos, 3>& b,
    PhysicsMaterial physics_material)
{
    OrderableEdgeSphere edge{
        .collision_line_sphere{
            .bounding_sphere{BoundingSphere<CompressedScenePos, 3>{FixedArray<CompressedScenePos, 2, 3>{a, b}}},
            .physics_material = physics_material,
            .line{a, b},
            .ray{a, b}} };
    edges_.insert(edge);
}

CollisionEdges::const_iterator CollisionEdges::begin() const {
    return edges_.begin();
}

CollisionEdges::const_iterator CollisionEdges::end() const {
    return edges_.end();
}

size_t CollisionEdges::size() const {
    return edges_.size();
}

namespace Mlib {
    template void CollisionEdges::insert<3>(
        const FixedArray<CompressedScenePos, 3, 3>& tri,
        PhysicsMaterial physics_material);
    template void CollisionEdges::insert<4>(
        const FixedArray<CompressedScenePos, 4, 3>& tri,
        PhysicsMaterial physics_material);
}
