#include "Collide_With_Movables.hpp"
#include <Mlib/Geometry/Mesh/Intersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Lines.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Reverse_Iterator.hpp>

using namespace Mlib;

static void collide_objects(
    const RigidBodyAndIntersectableMeshes& o0,
    const RigidBodyAndIntersectableMeshes& o1,
    const CollisionHistory& history)
{
    if (o0.rigid_body == o1.rigid_body) {
        throw std::runtime_error("Cannot collide identical objects");
    }
    if ((o0.rigid_body->mass() == INFINITY) && (o1.rigid_body->mass() == INFINITY)) {
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
            PhysicsMaterial combined_material = (msh0.physics_material | msh1.physics_material);
            if (any(combined_material & PhysicsMaterial::OBJ_BULLET_MASK) &&
               !any(combined_material & PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK))
            {
                continue;
            }
            if (!msh0.mesh->intersects(*msh1.mesh)) {
                continue;
            }
            for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                collide_triangle_and_triangles(
                    *o0.rigid_body,
                    *o1.rigid_body,
                    *msh0.mesh,
                    msh1,
                    t0,
                    history);
                collide_triangle_and_lines(
                    *o0.rigid_body,
                    *o1.rigid_body,
                    *msh0.mesh,
                    msh1,
                    t0,
                    history);
            }
            for (const auto& t1 : msh1.mesh->get_triangles_sphere()) {
                collide_triangle_and_triangles(
                    *o1.rigid_body,
                    *o0.rigid_body,
                    *msh1.mesh,
                    msh0,
                    t1,
                    history);
                collide_triangle_and_lines(
                    *o1.rigid_body,
                    *o0.rigid_body,
                    *msh1.mesh,
                    msh0,
                    t1,
                    history);
            }
        }
    }
}

void Mlib::collide_with_movables(
    CollisionDirection collision_direction,
    RigidBodies& rigid_bodies,
    const CollisionHistory& history)
{
    if (collision_direction == CollisionDirection::FORWARD) {
        size_t i0 = 0;
        for (const auto& o0 : rigid_bodies.transformed_objects()) {
            size_t i1 = 0;
            for (const auto& o1 : rigid_bodies.transformed_objects()) {
                if (i0 < i1) {
                    collide_objects(o0, o1, history);
                }
                ++i1;
            }
            ++i0;
        }
    } else {
        size_t i0 = 0;
        for (const auto& o0 : reverse(rigid_bodies.transformed_objects())) {
            size_t i1 = 0;
            for (const auto& o1 : reverse(rigid_bodies.transformed_objects())) {
                if (i0 < i1) {
                    collide_objects(o0, o1, history);
                }
                ++i1;
            }
            ++i0;
        }
    }
}
