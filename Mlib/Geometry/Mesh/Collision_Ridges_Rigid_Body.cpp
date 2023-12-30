#include "Collision_Ridges_Rigid_Body.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

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
    for (size_t i = 0; i < 3; ++i) {
        size_t j = (i + 1) % 3;
        try {
            insert(tri(i), tri(j), normal, max_min_cos_ridge, physics_material, rb);
        } catch (const EdgeException<double>& e) {
            throw TriangleEdgeException<double>{
                tri(0), tri(1), tri(2), i, j, e.what()};
        }
    }
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
                .ray{a, b},
                .normal = normal,
                .min_cos = RIDGE_SINGLE_FACE}
        },
        rb};
    CollisionRidgesBase<OrderableRidgeSphereRigidBody>::insert(ridge, max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereRigidBody>;
}
