#include "Collide_Intersectables_And_Intersectables.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>

using namespace Mlib;

void Mlib::collide_intersectables_and_intersectables(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionHistory& history)
{
    const auto& intersectables0 = msh0.mesh->get_intersectables();
    if (intersectables0.empty()) {
        return;
    }
    const auto& intersectables1 = msh1.mesh->get_intersectables();
    if (intersectables1.empty()) {
        return;
    }
    if (!msh0.mesh->bounding_sphere().intersects(msh1.mesh->bounding_sphere())) {
        return;
    }
    auto non_tire_line_mask =
        PhysicsMaterial::OBJ_CHASSIS |
        PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
        PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    if (any(msh1.physics_material & non_tire_line_mask)) {
        for (const auto& i0 : intersectables0) {
            for (const auto& i1 : intersectables1) {
                if (!i1.mesh->bounding_sphere().intersects(msh0.mesh->bounding_sphere())) {
                    continue;
                }
                handle_line_triangle_intersection(IntersectionScene{
                    .o0 = o0,
                    .o1 = o1,
                    .mesh0 = msh0.mesh.get(),
                    .mesh1 = msh1.mesh.get(),
                    .l1 = std::nullopt,
                    .r1 = std::nullopt,
                    .i1 = i1.mesh.get(),
                    .q0 = std::nullopt,
                    .t0 = std::nullopt,
                    .i0 = i0.mesh.get(),
                    .tire_id1 = SIZE_MAX,
                    .mesh0_material = msh0.physics_material,
                    .mesh1_material = msh1.physics_material,
                    .l1_is_normal = false,
                    .surface_contact_info = history.surface_contact_db.get_contact_info(
                        msh0.physics_material,
                        msh1.physics_material,
                        SIZE_MAX),
                    .default_collision_type = CollisionType::REFLECT,
                    .history = history });
            }
        }
    }
}
