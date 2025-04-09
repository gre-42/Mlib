#include "Collide_Triangle_And_Intersectables.hpp"
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

void Mlib::collide_triangle_and_intersectables(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<CompressedScenePos, 4>* q0,
    const CollisionPolygonSphere<CompressedScenePos, 3>* t0,
    const CollisionHistory& history)
{
    const auto& intersectables1 = msh1.mesh->get_intersectables();
    if (intersectables1.empty()) {
        return;
    }
    auto non_tire_line_mask =
        PhysicsMaterial::OBJ_CHASSIS |
        PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
        PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    auto collide = [&](const auto& poly0){
        if (any(msh1.physics_material & non_tire_line_mask)) {
            for (const auto& i1 : intersectables1) {
                if (!i1.mesh->bounding_sphere().intersects(poly0.bounding_sphere)) {
                    continue;
                }
                handle_line_triangle_intersection(IntersectionScene{
                    .o0 = o0,
                    .o1 = o1,
                    .mesh0 = nullptr,
                    .mesh1 = msh1.mesh.get(),
                    .l1 = std::nullopt,
                    .r1 = std::nullopt,
                    .i1 = i1.mesh.get(),
                    .q0 = pointer_to_optional(q0),
                    .t0 = pointer_to_optional(t0),
                    .i0 = nullptr,
                    .tire_id1 = SIZE_MAX,
                    .mesh0_material = poly0.physics_material,
                    .mesh1_material = msh1.physics_material,
                    .l1_is_normal = false,
                    .surface_contact_info = history.surface_contact_db.get_contact_info(
                        poly0.physics_material,
                        msh1.physics_material,
                        SIZE_MAX),
                    .default_collision_type = CollisionType::REFLECT,
                    .history = history});
            }
        }
    };
    try
    {
        if (q0 != nullptr) {
            collide(*q0);
        }
        if (t0 != nullptr) {
            collide(*t0);
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Error colliding objects \"" + o0.name() + "\" and \"" + o1.name() + "\": " + e.what());
    }
}

void Mlib::collide_triangle_and_intersectables(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<CompressedScenePos, 3>, CollisionPolygonSphere<CompressedScenePos, 4>>& cps0,
    const CollisionHistory& history)
{
    collide_triangle_and_intersectables(
        o0,
        o1,
        msh1,
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 4>>(&cps0),
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 3>>(&cps0),
        history);
}
