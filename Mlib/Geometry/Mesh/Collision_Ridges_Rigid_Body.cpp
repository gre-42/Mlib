#include "Collision_Ridges_Rigid_Body.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>

using namespace Mlib;

CollisionRidgesRigidBody::CollisionRidgesRigidBody() = default;

CollisionRidgesRigidBody::~CollisionRidgesRigidBody() = default;

void CollisionRidgesRigidBody::insert(
    const FixedArray<FixedArray<double, 3>, 3>& tri,
    const FixedArray<double, 3>& normal,
    double max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    insert(tri(0), tri(1), normal, max_min_cos_ridge, physics_material, rb);
    insert(tri(1), tri(2), normal, max_min_cos_ridge, physics_material, rb);
    insert(tri(2), tri(0), normal, max_min_cos_ridge, physics_material, rb);
}

void CollisionRidgesRigidBody::insert(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& normal,
    double max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    OrderableRidgeSphereRigidBody ridge{
        {
            .collision_ridge_sphere{
                .bounding_sphere{BoundingSphere<double, 3>{FixedArray<FixedArray<double, 3>, 2>{a, b}}},
                .physics_material = physics_material,
                .edge{a, b},
                .normal = normal,
                .min_cos = NAN}
        },
        rb};
    CollisionRidgesBase<OrderableRidgeSphereRigidBody>::insert(ridge, max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereRigidBody>;
}
