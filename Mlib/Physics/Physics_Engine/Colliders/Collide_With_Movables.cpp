#include "Collide_With_Movables.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Iterator/Reverse_Iterator.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Convex_Meshes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static void collide_objects(
    const RigidBodyAndIntersectableMeshes& o0,
    const RigidBodyAndIntersectableMeshes& o1,
    const CollisionHistory& history)
{
    if (&o0.rigid_body == &o1.rigid_body) {
        THROW_OR_ABORT("Cannot collide identical objects");
    }
    if ((o0.rigid_body->mass() == INFINITY) && (o1.rigid_body->mass() == INFINITY)) {
        return;
    }
    if (!history.phase.group.rigid_bodies.contains(&o0.rigid_body->rbp_) &&
        !history.phase.group.rigid_bodies.contains(&o1.rigid_body->rbp_))
    {
        return;
    }
    if (o0.rigid_body->non_colliders_.contains(o1.rigid_body) ||
        o1.rigid_body->non_colliders_.contains(o0.rigid_body))
    {
        return;
    }
    PhysicsMaterial included_materials =
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK |
        PhysicsMaterial::OBJ_BULLET_MASK |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    for (const auto& msh1 : o1.meshes) {
        if (!any(msh1.physics_material & included_materials)) {
            continue;
        }
        for (const auto& msh0 : o0.meshes) {
            if (!any(msh0.physics_material & included_materials)) {
                continue;
            }
            collide_convex_meshes(
                o0.rigid_body.get(),
                o1.rigid_body.get(),
                msh0,
                msh1,
                history);
        }
    }
}

void Mlib::collide_with_movables(
    CollisionDirection collision_direction,
    RigidBodies& rigid_bodies,
    const CollisionHistory& history)
{
    if (collision_direction == CollisionDirection::FORWARD) {
        for (const auto& [i0, o0] : enumerate(rigid_bodies.transformed_objects())) {
            for (const auto& [i1, o1] : enumerate(rigid_bodies.transformed_objects())) {
                if (i1 >= i0) {
                    break;
                }
                collide_objects(o0, o1, history);
            }
        }
    } else {
        for (const auto& [i0, o0] : enumerate(reverse(rigid_bodies.transformed_objects()))) {
            for (const auto& [i1, o1] : enumerate(reverse(rigid_bodies.transformed_objects()))) {
                if (i1 >= i0) {
                    break;
                }
                collide_objects(o0, o1, history);
            }
        }
    }
}
