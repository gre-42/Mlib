#include "Collide_Triangles_And_Ridge.hpp"
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
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
    auto collide = [&](
        const auto& poly0,
        const CollisionPolygonSphere<4>* q0,
        const CollisionPolygonSphere<3>* t0)
    {
        if (!any(poly0.physics_material & non_tire_line_mask)) {
            return;
        }
        if (!r1.bounding_sphere.intersects(poly0.bounding_sphere)) {
            return;
        }
        handle_line_triangle_intersection(IntersectionScene{
            .o0 = o0,
            .o1 = o1,
            .mesh0 = msh0.mesh.get(),
            .mesh1 = nullptr,
            .l1 = nullptr,
            .r1 = &r1,
            .q0 = q0,
            .t0 = t0,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = poly0.physics_material,
            .mesh1_material = r1.physics_material,
            .l1_is_normal = false,
            .default_collision_type = CollisionType::REFLECT,
            .history = history});
    };
    for (const auto& q0 : msh0.mesh->get_quads_sphere()) {
        collide(q0, &q0, nullptr);
    }
    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
        collide(t0, nullptr, &t0);
    }
}
