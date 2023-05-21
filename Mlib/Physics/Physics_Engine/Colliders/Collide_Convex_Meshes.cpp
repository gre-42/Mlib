#include "Collide_Convex_Meshes.hpp"
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Lines.hpp>
#include <Mlib/Physics/Collision/Detect/Collide_Triangle_And_Triangles.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>

using namespace Mlib;

void Mlib::collide_convex_meshes(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionHistory& history)
{
    PhysicsMaterial combined_material = (msh0.physics_material | msh1.physics_material);
    if (any(combined_material & PhysicsMaterial::OBJ_BULLET_MASK) &&
       !any(combined_material & PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK))
    {
        return;
    }
    if (!any(combined_material & PhysicsMaterial::OBJ_BULLET_MASK) &&
         any(combined_material & PhysicsMaterial::OBJ_HITBOX))
    {
        return;
    }
    if (!msh0.mesh->intersects(*msh1.mesh)) {
        return;
    }
    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
        collide_triangle_and_triangles(
            o0,
            o1,
            msh0.mesh.get(),
            msh1,
            t0,
            history);
        collide_triangle_and_lines(
            o0,
            o1,
            msh1,
            t0,
            history);
    }
    for (const auto& t1 : msh1.mesh->get_triangles_sphere()) {
        collide_triangle_and_triangles(
            o1,
            o0,
            msh1.mesh.get(),
            msh0,
            t1,
            history);
        collide_triangle_and_lines(
            o1,
            o0,
            msh0,
            t1,
            history);
    }
}
