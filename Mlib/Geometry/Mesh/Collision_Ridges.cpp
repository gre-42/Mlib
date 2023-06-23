#include "Collision_Ridges.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>

using namespace Mlib;

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
    OrderableRidgeSphereBase ridge{
        .collision_ridge_sphere{
            .bounding_sphere{BoundingSphere<double, 3>{FixedArray<FixedArray<double, 3>, 2>{a, b}}},
            .physics_material = physics_material,
            .edge{a, b},
            .normal = normal,
            .min_cos = NAN}};
    CollisionRidgesBase<OrderableRidgeSphereBase>::insert(ridge, max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereBase>;
}
