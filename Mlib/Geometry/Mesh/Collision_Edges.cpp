#include "Collision_Edges.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>> OrderableEdgeSphere::key() const
{
    if (OrderableFixedArray{collision_line_sphere.line(0)} > OrderableFixedArray{collision_line_sphere.line(1)}) {
        return std::make_pair(
            OrderableFixedArray{collision_line_sphere.line(0)},
            OrderableFixedArray{collision_line_sphere.line(1)});
    } else {
        return std::make_pair(
            OrderableFixedArray{collision_line_sphere.line(1)},
            OrderableFixedArray{collision_line_sphere.line(0)});
    }
}

bool OrderableEdgeSphere::operator < (const OrderableEdgeSphere& other) const {
    return key() < other.key();
}

CollisionEdges::CollisionEdges() = default;

CollisionEdges::~CollisionEdges() = default;

void CollisionEdges::insert(
    const FixedArray<FixedArray<double, 3>, 3>& tri,
    PhysicsMaterial physics_material)
{
    insert(tri(0), tri(1), physics_material);
    insert(tri(1), tri(2), physics_material);
    insert(tri(2), tri(0), physics_material);
}

void CollisionEdges::insert(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    PhysicsMaterial physics_material)
{
    OrderableEdgeSphere edge{
        .collision_line_sphere{
            .bounding_sphere{BoundingSphere<double, 3>{FixedArray<FixedArray<double, 3>, 2>{a, b}}},
            .physics_material = physics_material,
            .line{a, b}}};
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
