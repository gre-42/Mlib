#include "Collide_With_Terrain.hpp"
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Line_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Edges.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Intersectables.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Lines.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangles_And_Ridge.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Convex_Meshes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {
    
inline bool intersects(
    const AxisAlignedBoundingBox<ScenePos, 3>& a,
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& b)
{
    return a.intersects(b.casted<ScenePos>());
}

}

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
            if (!history.phase.group.rigid_bodies.contains(&o1.rigid_body->rbp_)) {
                continue;
            }
            PhysicsMaterial collide_with_terrain_triangle_mask =
                PhysicsMaterial::OBJ_CHASSIS |
                PhysicsMaterial::OBJ_TIRE_LINE |
                PhysicsMaterial::OBJ_BULLET_MASK |
                PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
                PhysicsMaterial::OBJ_DISTANCEBOX;
            if (any(msh1.physics_material & collide_with_terrain_triangle_mask)) {
                if (any(msh1.physics_material & PhysicsMaterial::ATTR_CONVEX) ||
                    any(msh1.physics_material & PhysicsMaterial::OBJ_TIRE_LINE) ||
                    any(msh1.physics_material & PhysicsMaterial::OBJ_BULLET_MASK))
                {
                    rigid_bodies.convex_mesh_bvh().grid().visit(
                        msh1.mesh->aabb(),
                        [&](const RigidBodyAndIntersectableMesh& rm) {
                            collide_convex_meshes(
                                rm.rb.get(),
                                o1.rigid_body.get(),
                                rm.mesh,
                                msh1,
                                history);
                            return true;
                        });
                }
                rigid_bodies.triangle_bvh().grid().visit(
                    msh1.mesh->aabb(),
                    [&](const RigidBodyAndCollisionTriangleSphere<CompressedScenePos>& t0){
                        return std::visit([&](const auto& ctp)
                            {
                                if (any(ctp.physics_material & PhysicsMaterial::ATTR_CONVEX) &&
                                    any(msh1.physics_material & PhysicsMaterial::ATTR_CONVEX))
                                {
                                    return true;
                                }
                                if (any(msh1.physics_material & PhysicsMaterial::OBJ_BULLET_MESH) &&
                                    !any(msh1.physics_material & PhysicsMaterial::ATTR_CONVEX))
                                {
                                    collide_triangle_and_triangles(
                                        t0.rb,
                                        o1.rigid_body.get(),
                                        nullptr,
                                        msh1,
                                        ctp,
                                        history);
                                }
                                collide_triangle_and_edges(
                                    t0.rb,
                                    o1.rigid_body.get(),
                                    msh1,
                                    ctp,
                                    history);
                                collide_triangle_and_lines(
                                    t0.rb,
                                    o1.rigid_body.get(),
                                    msh1,
                                    ctp,
                                    history);
                                collide_triangle_and_intersectables(
                                    t0.rb,
                                    o1.rigid_body.get(),
                                    msh1,
                                    ctp,
                                    history);
                                return true;
                            },
                            t0.ctp);
                    });
                rigid_bodies.ridge_bvh().visit(
                    msh1.mesh->aabb(),
                    [&](const RigidBodyAndCollisionRidgeSphere<CompressedScenePos>& e0){
                        collide_triangles_and_ridge(
                            o1.rigid_body.get(),
                            e0.rb,
                            msh1,
                            e0.crp,
                            history);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_GRIND_CONTACT)) {
                rigid_bodies.line_bvh().visit(
                    msh1.mesh->aabb(),
                    [&](const RigidBodyAndCollisionLineSphere<CompressedScenePos>& l0){
                        collide_line_and_triangles(
                            l0.rb,
                            o1.rigid_body.get(),
                            *msh1.mesh,
                            l0.clp,
                            history);
                        return true;
                    });
            } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
                if (!msh1.mesh->get_lines_sphere().empty()) {
                    THROW_OR_ABORT("Detected hitbox with lines in object \"" + o1.rigid_body->name() + '"');
                }
            } else {
                THROW_OR_ABORT(
                    "Unknown mesh type when colliding object \"" + o1.rigid_body->name() + '"');
            }
        }
    }
}
