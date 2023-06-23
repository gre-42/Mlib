#include "Collision_Ridges.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>> OrderableRidgeSphere::key() const
{
    if (OrderableFixedArray{collision_ridge_sphere.edge(0)} > OrderableFixedArray{collision_ridge_sphere.edge(1)}) {
        return std::make_pair(
            OrderableFixedArray{collision_ridge_sphere.edge(0)},
            OrderableFixedArray{collision_ridge_sphere.edge(1)});
    } else {
        return std::make_pair(
            OrderableFixedArray{collision_ridge_sphere.edge(1)},
            OrderableFixedArray{collision_ridge_sphere.edge(0)});
    }
}

bool OrderableRidgeSphere::operator < (const OrderableRidgeSphere& other) const {
    return key() < other.key();
}

CollisionRidges::CollisionRidges() = default;

CollisionRidges::~CollisionRidges() = default;

void CollisionRidges::insert(
    const FixedArray<FixedArray<double, 3>, 3>& tri,
    const FixedArray<double, 3>& normal,
    double max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    insert(tri(0), tri(1), normal, max_min_cos_ridge, physics_material);
    insert(tri(1), tri(2), normal, max_min_cos_ridge, physics_material);
    insert(tri(2), tri(0), normal, max_min_cos_ridge, physics_material);
}

void CollisionRidges::insert(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& normal,
    double max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    OrderableRidgeSphere edge{
        .collision_ridge_sphere{
            .bounding_sphere{BoundingSphere<double, 3>{FixedArray<FixedArray<double, 3>, 2>{a, b}}},
            .physics_material = physics_material,
            .edge{a, b}}};
    auto previous_edge = edges_.find(edge);
    if (previous_edge == edges_.end()) {
        edge.collision_ridge_sphere.normal = normal;
        edge.collision_ridge_sphere.min_cos = NAN;
        if (!edges_.insert(edge).second) {
            THROW_OR_ABORT("CollisionRidges::insert internal error");
        }
    } else {
        auto& old_edge = const_cast<CollisionRidgeSphere&>(previous_edge->collision_ridge_sphere);
        if (!std::isnan(old_edge.min_cos)) {
            THROW_OR_ABORT("Detected duplicate triangles touching the same edge");
        }
        if (dot0d(cross(old_edge.edge(1) - old_edge.edge(0), old_edge.normal), normal) < 0) {
            edges_.erase(previous_edge);
            return;
        }
        auto average_normal = (normal + old_edge.normal);
        auto len2 = sum(squared(average_normal));
        if (len2 < 1e-7) {
            THROW_OR_ABORT("Detected parallel triangles with opposing faces");
        }
        old_edge.normal = average_normal / std::sqrt(len2);
        old_edge.min_cos = dot0d(old_edge.normal, normal);
        if (old_edge.min_cos > max_min_cos_ridge) {
            edges_.erase(previous_edge);
        }
    }
}

CollisionRidges::const_iterator CollisionRidges::begin() const {
    return edges_.begin();
}

CollisionRidges::const_iterator CollisionRidges::end() const {
    return edges_.end();
}

size_t CollisionRidges::size() const {
    return edges_.size();
}
