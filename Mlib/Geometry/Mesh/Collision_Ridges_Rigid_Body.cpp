#include "Collision_Ridges_Rigid_Body.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

using namespace Mlib;

CollisionRidgesRigidBody::CollisionRidgesRigidBody() = default;

CollisionRidgesRigidBody::~CollisionRidgesRigidBody() = default;

template <size_t tnvertices>
void CollisionRidgesRigidBody::insert(
    const FixedArray<FixedArray<ScenePos, 3>, tnvertices>& polygon,
    const FixedArray<ScenePos, 3>& normal,
    const FixedArray<FixedArray<float, 3>, tnvertices>& vertex_normals,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        size_t j = (i + 1) % tnvertices;
        try {
            insert(
                polygon(i),
                polygon(j),
                normal,
                vertex_normals(i),
                vertex_normals(j),
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
    const FixedArray<float, 3>& a_vertex_normal,
    const FixedArray<float, 3>& b_vertex_normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material,
    RigidBodyVehicle& rb)
{
    OrderableRidgeSphereRigidBody ridge{
        {
            CollisionRidgeSphere{
                .bounding_sphere{BoundingSphere<ScenePos, 3>{FixedArray<FixedArray<ScenePos, 3>, 2>{a, b}}},
                .physics_material = physics_material,
                .edge{a, b},
                .ray{a, b},
                .normal = normal,
                .vertex_normals = {a_vertex_normal, b_vertex_normal},
                .min_cos = RIDGE_SINGLE_FACE}
        },
        rb};
    CollisionRidgesBase<OrderableRidgeSphereRigidBody>::insert(ridge, max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereRigidBody>;
    template void CollisionRidgesRigidBody::insert<3>(
        const FixedArray<FixedArray<ScenePos, 3>, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        const FixedArray<FixedArray<float, 3>, 3>& vertex_normals,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
    template void CollisionRidgesRigidBody::insert<4>(
        const FixedArray<FixedArray<ScenePos, 3>, 4>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        const FixedArray<FixedArray<float, 3>, 4>& vertex_normals,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
}
