#include "Collide_Triangles_And_Ridge.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Pointer_To_Optional.hpp>

using namespace Mlib;

void Mlib::collide_triangles_and_ridge(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
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
        PhysicsMaterial physics_material0,
        const BoundingSphere<CompressedScenePos, 3>& bounding_sphere0,
        const CollisionPolygonSphere<CompressedScenePos, 4>* q0,
        const CollisionPolygonSphere<CompressedScenePos, 3>* t0,
        const IIntersectable* i0)
    {
        if (!any(physics_material0 & non_tire_line_mask)) {
            return;
        }
        if (!r1.bounding_sphere.intersects(bounding_sphere0)) {
            return;
        }
        handle_line_triangle_intersection(IntersectionScene{
            .o0 = o0,
            .o1 = o1,
            .mesh0 = msh0.mesh.get(),
            .mesh1 = nullptr,
            .l1 = std::nullopt,
            .r1 = r1,
            .q0 = pointer_to_optional(q0),
            .t0 = pointer_to_optional(t0),
            .i0 = i0,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = physics_material0,
            .mesh1_material = r1.physics_material,
            .l1_is_normal = false,
            .surface_contact_info = history.surface_contact_db.get_contact_info(
                physics_material0,
                r1.physics_material),
            .default_collision_type = CollisionType::REFLECT,
            .history = history});
    };
    for (const auto& q0 : msh0.mesh->get_quads_sphere()) {
        collide(q0.physics_material, q0.bounding_sphere, &q0, nullptr, nullptr);
    }
    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
        collide(t0.physics_material, t0.bounding_sphere, nullptr, &t0, nullptr);
    }
    for (const auto& i0 : msh0.mesh->get_intersectables()) {
        collide(i0.physics_material, i0.mesh->bounding_sphere(), nullptr, nullptr, i0.mesh.get());
    }
}
