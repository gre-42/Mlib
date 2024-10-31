#include "Collision_Ridges_Rigid_Body.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

using namespace Mlib;

CollisionRidgesRigidBody::CollisionRidgesRigidBody() = default;

CollisionRidgesRigidBody::~CollisionRidgesRigidBody() = default;

template <size_t tnvertices>
void CollisionRidgesRigidBody::insert(
    const FixedArray<ScenePos, tnvertices, 3>& polygon,
    const FixedArray<ScenePos, 3>& normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        size_t j = (i + 1) % tnvertices;
        try {
            insert(
                polygon[i],
                polygon[j],
                normal,
                max_min_cos_ridge,
                physics_material,
                rb);
        } catch (const EdgeException<ScenePos>& e) {
            throw PolygonEdgeException<ScenePos, tnvertices>{
                polygon, i, j, e.what()};
        }
    }
}

void CollisionRidgesRigidBody::insert(
    const FixedArray<ScenePos, 3>& a,
    const FixedArray<ScenePos, 3>& b,
    const FixedArray<ScenePos, 3>& normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    OrderableRidgeSphereRigidBody ridge{
        {
            CollisionRidgeSphere<ScenePos>{
                .bounding_sphere{BoundingSphere<ScenePos, 3>{FixedArray<ScenePos, 2, 3>{a, b}}},
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
    template void CollisionRidgesRigidBody::insert<3>(
        const FixedArray<ScenePos, 3, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
    template void CollisionRidgesRigidBody::insert<4>(
        const FixedArray<ScenePos, 4, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
}
