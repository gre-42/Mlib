#include "Collide_Triangle_And_Lines.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Increment_In_Destructor.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::collide_triangle_and_lines(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionTriangleSphere& t0,
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
    if (any(msh1.physics_material & non_tire_line_mask)) {
        for (const auto& l1 : lines1) {
            if (!l1.bounding_sphere.intersects(t0.bounding_sphere)) {
                continue;
            }
            if (!l1.bounding_sphere.intersects(t0.plane)) {
                continue;
            }
            handle_line_triangle_intersection(IntersectionScene{
                .o0 = o0,
                .o1 = o1,
                .mesh0 = nullptr,
                .mesh1 = msh1.mesh.get(),
                .l1 = &l1,
                .r1 = nullptr,
                .t0 = t0,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = true,
                .default_collision_type = CollisionType::REFLECT,
                .history = history});
        }
    } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_TIRE_LINE)) {
        size_t tire_id1 = 0;
        for (const auto& l1 : lines1) {
            IncrementInDestructor iid{tire_id1};
            if (!l1.bounding_sphere.intersects(t0.bounding_sphere)) {
                continue;
            }
            if (!l1.bounding_sphere.intersects(t0.plane)) {
                continue;
            }
            handle_line_triangle_intersection(IntersectionScene{
                .o0 = o0,
                .o1 = o1,
                .mesh0 = nullptr,
                .mesh1 = msh1.mesh.get(),
                .l1 = &l1,
                .r1 = nullptr,
                .t0 = t0,
                .tire_id1 = tire_id1,
                .mesh0_material = t0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = true,
                .default_collision_type = CollisionType::REFLECT,
                .history = history});
        }
        if (tire_id1 != o1.tires_.size()) {
            THROW_OR_ABORT(
                "Number of tire-lines (" + std::to_string(tire_id1) + ") does not equal the "
                "number of tires (" + std::to_string(o1.tires_.size()) + ") in object \"" + o1.name() + '"');
        }
    } else if (any(msh1.physics_material & PhysicsMaterial::OBJ_HITBOX)) {
        THROW_OR_ABORT("Detected hitbox with lines in object \"" + o1.name() + '"');
    } else {
        THROW_OR_ABORT(
            "Unknown mesh type when colliding objects \"" +
            o0.name() + "\" and \"" + o1.name() + '"');
    }
}
