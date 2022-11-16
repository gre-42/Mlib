#include "Collide_With_Terrain.hpp"
#include <Mlib/Geometry/Mesh/Intersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_History.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Line_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Lines.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

void Mlib::collide_with_terrain(
    RigidBodies& rigid_bodies,
    const CollisionHistory& history)
{
    for (const auto& o1 : rigid_bodies.transformed_objects()) {
        if (o1.rigid_body->mass() == INFINITY) {
            continue;
        }
        for (const auto& msh1 : o1.meshes) {
            PhysicsMaterial collide_with_terrain_triangle_mask =
                PhysicsMaterial::OBJ_CHASSIS |
                PhysicsMaterial::OBJ_TIRE_LINE |
                PhysicsMaterial::OBJ_BULLET_MASK |
                PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
                PhysicsMaterial::OBJ_DISTANCEBOX;
            if (any(msh1.physics_material & collide_with_terrain_triangle_mask)) {
                rigid_bodies.triangle_bvh().visit(
                    msh1.mesh->aabb(),
                    [&](const RigidBodyAndCollisionTriangleSphere& t0){
                        if (any(msh1.physics_material & PhysicsMaterial::OBJ_BULLET_MESH) ||
                            (any(t0.ctp.physics_material & PhysicsMaterial::ATTR_CONVEX) &&
                             any(msh1.physics_material & PhysicsMaterial::ATTR_CONVEX)))
                        {
                            collide_triangle_and_triangles(
                                t0.rb,
                                *o1.rigid_body,
                                *t0.mesh,
                                msh1,
                                t0.ctp,
                                history);
                        }
                        collide_triangle_and_lines(
                            t0.rb,
                            *o1.rigid_body,
                            *t0.mesh,
                            msh1,
                            t0.ctp,
                            history);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_GRIND_CONTACT)) {
                rigid_bodies.line_bvh().visit(
                    msh1.mesh->aabb(),
                    [&](const RigidBodyAndCollisionLineSphere& l0){
                        collide_line_and_triangles(
                            l0.rb,
                            *o1.rigid_body,
                            *msh1.mesh,
                            l0.clp,
                            history);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
                if (!msh1.mesh->get_lines_sphere().empty()) {
                    throw std::runtime_error("Detected hitbox with lines in object \"" + o1.rigid_body->name() + '"');
                }
            } else {
                throw std::runtime_error(
                    "Unknown mesh type when colliding object \"" + o1.rigid_body->name() + '"');
            }
        }
    }
}
