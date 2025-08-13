#include "Collide_Triangle_And_Lines.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Pointer_To_Optional.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::collide_triangle_and_lines(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<CompressedScenePos, 4>* q0,
    const CollisionPolygonSphere<CompressedScenePos, 3>* t0,
    const CollisionHistory& history)
{
    const auto& lines1 = msh1.mesh->get_lines_sphere();
    if (lines1.empty()) {
        return;
    }
    auto non_tire_line_mask =
        PhysicsMaterial::OBJ_CHASSIS |
        PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
        PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    auto collide = [&](
        const auto& bounding_sphere0,
        PhysicsMaterial physics_material0)
    {
        if (any(msh1.physics_material & non_tire_line_mask)) {
            for (const auto& l1 : lines1) {
                if (!l1.bounding_sphere.intersects(bounding_sphere0)) {
                    continue;
                }
                handle_line_triangle_intersection(IntersectionScene{
                    .o0 = o0,
                    .o1 = o1,
                    .mesh0 = nullptr,
                    .mesh1 = msh1.mesh.get(),
                    .l1 = l1,
                    .r1 = std::nullopt,
                    .q0 = pointer_to_optional(q0),
                    .t0 = pointer_to_optional(t0),
                    .tire_id1 = SIZE_MAX,
                    .mesh0_material = physics_material0,
                    .mesh1_material = msh1.physics_material,
                    .l1_is_normal = true,
                    .surface_contact_info = history.surface_contact_db.get_contact_info(
                        physics_material0,
                        msh1.physics_material),
                    .default_collision_type = CollisionType::REFLECT,
                    .history = history});
            }
        } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_TIRE_LINE)) {
            if (lines1.size() != o1.tires_.size()) {
                THROW_OR_ABORT(
                    "Number of tire-lines (" + std::to_string(lines1.size()) + ") does not equal the "
                    "number of tires (" + std::to_string(o1.tires_.size()) + ") in object \"" + o1.name() + '"');
            }
            for (const auto& [tire_id1, l1] : enumerate(lines1)) {
                if (!l1.bounding_sphere.intersects(bounding_sphere0)) {
                    continue;
                }
                handle_line_triangle_intersection(IntersectionScene{
                    .o0 = o0,
                    .o1 = o1,
                    .mesh0 = nullptr,
                    .mesh1 = msh1.mesh.get(),
                    .l1 = l1,
                    .r1 = std::nullopt,
                    .q0 = pointer_to_optional(q0),
                    .t0 = pointer_to_optional(t0),
                    .tire_id1 = tire_id1,
                    .mesh0_material = physics_material0,
                    .mesh1_material = msh1.physics_material,
                    .l1_is_normal = true,
                    .surface_contact_info = history.surface_contact_db.get_contact_info(
                        physics_material0,
                        msh1.physics_material),
                    .default_collision_type = CollisionType::REFLECT,
                    .history = history});
            }
        } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
            THROW_OR_ABORT("Detected hitbox with lines in object \"" + o1.name() + '"');
        } else {
            THROW_OR_ABORT(
                "Unknown mesh type when colliding objects \"" +
                o0.name() + "\" and \"" + o1.name() + '"');
        }
    };
    if (q0 != nullptr) {
        collide(q0->bounding_sphere, q0->physics_material);
    }
    if (t0 != nullptr) {
        collide(t0->bounding_sphere, t0->physics_material);
    }
}

void Mlib::collide_triangle_and_lines(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<CompressedScenePos, 3>, CollisionPolygonSphere<CompressedScenePos, 4>>& cps0,
    const CollisionHistory& history)
{
    collide_triangle_and_lines(
        o0,
        o1,
        msh1,
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 4>>(&cps0),
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 3>>(&cps0),
        history);
}
