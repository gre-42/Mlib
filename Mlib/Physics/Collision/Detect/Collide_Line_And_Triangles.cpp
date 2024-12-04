#include "Collide_Line_And_Triangles.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>

using namespace Mlib;

void Mlib::collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh& msh1,
    const CollisionLineSphere<CompressedScenePos>& l0,
    const CollisionHistory& history)
{
    // Mesh-sphere <-> line-sphere intersection
    if (!msh1.intersects(l0.bounding_sphere)) {
        return;
    }
    for (const auto& t1 : msh1.get_triangles_sphere()) {
        if (!t1.bounding_sphere.intersects(l0.bounding_sphere)) {
            continue;
        }
        if (!l0.bounding_sphere.intersects(t1.polygon.plane)) {
            continue;
        }
        handle_line_triangle_intersection(IntersectionScene{
            .o0 = o1,
            .o1 = o0,
            .mesh0 = nullptr, // msh1.mesh,
            .mesh1 = nullptr, // msh0,
            .l1 = l0,
            .r1 = std::nullopt,
            .q0 = std::nullopt,
            .t0 = t1,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = t1.physics_material,
            .mesh1_material = l0.physics_material,
            .l1_is_normal = false,
            .surface_contact_info = history.surface_contact_db.get_contact_info(
                l0.physics_material,
                t1.physics_material,
                SIZE_MAX),
            .default_collision_type = CollisionType::GRIND,
            .history = history});
    }
}
