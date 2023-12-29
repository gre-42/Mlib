#include "Collide_Triangles_And_Ridge.hpp"
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>

using namespace Mlib;

void Mlib::collide_triangles_and_ridge(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const CollisionRidgeSphere& r1,
    const CollisionHistory& history)
{
    auto non_tire_line_mask =
        PhysicsMaterial::OBJ_CHASSIS |
        PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
        PhysicsMaterial::OBJ_ALIGNMENT_CONTACT |
        PhysicsMaterial::OBJ_DISTANCEBOX;
    if (!r1.bounding_sphere.intersects(msh0.mesh->bounding_sphere())) {
        return;
    }
    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
        if (!any(t0.physics_material & non_tire_line_mask)) {
            continue;
        }
        if (!r1.bounding_sphere.intersects(t0.bounding_sphere)) {
            continue;
        }
        if (!r1.bounding_sphere.intersects(t0.plane)) {
            continue;
        }
        handle_line_triangle_intersection(IntersectionScene{
            .o0 = o0,
            .o1 = o1,
            .mesh0 = msh0.mesh.get(),
            .mesh1 = nullptr,
            .l1 = nullptr,
            .r1 = &r1,
            .t0 = t0,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = t0.physics_material,
            .mesh1_material = r1.physics_material,
            .l1_is_normal = false,
            .default_collision_type = CollisionType::REFLECT,
            .history = history});
    }
}
