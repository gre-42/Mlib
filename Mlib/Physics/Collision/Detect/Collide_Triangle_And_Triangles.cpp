#include "Collide_Triangle_And_Triangles.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Pointer_To_Optional.hpp>

using namespace Mlib;

void Mlib::collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<CompressedScenePos, 4>* q0,
    const CollisionPolygonSphere<CompressedScenePos, 3>* t0,
    const CollisionHistory& history)
{
    auto collide = [&](
        const auto& bounding_sphere0,
        const PlaneNd<SceneDir, CompressedScenePos, 3>* plane0,
        PhysicsMaterial physics_material0)
    {
        // Mesh-sphere <-> triangle-sphere intersection
        if (!msh1.mesh->intersects(bounding_sphere0)) {
            return;
        }
        // Mesh-sphere <-> triangle-plane intersection
        if ((plane0 != nullptr) && !msh1.mesh->intersects(*plane0)) {
            return;
        }
        for (const auto& r1 : msh1.mesh->get_ridges_sphere()) {
            if (!r1.bounding_sphere.intersects(bounding_sphere0)) {
                continue;
            }
            handle_line_triangle_intersection(IntersectionScene{
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0,
                .mesh1 = msh1.mesh.get(),
                .l1 = std::nullopt,
                .r1 = r1,
                .q0 = pointer_to_optional(q0),
                .t0 = pointer_to_optional(t0),
                .tire_id1 = SIZE_MAX,
                .mesh0_material = physics_material0,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = false,
                .surface_contact_info = history.surface_contact_db.get_contact_info(
                    physics_material0,
                    msh1.physics_material),
                .default_collision_type = CollisionType::REFLECT,
                .history = history});
        }
    };
    if (q0 != nullptr) {
        collide(q0->bounding_sphere, &q0->polygon.plane, q0->physics_material);
    }
    if (t0 != nullptr) {
        collide(t0->bounding_sphere, &t0->polygon.plane, t0->physics_material);
    }
}

void Mlib::collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<CompressedScenePos, 3>, CollisionPolygonSphere<CompressedScenePos, 4>>& cps0,
    const CollisionHistory& history)
{
    collide_triangle_and_triangles(
        o0,
        o1,
        msh0,
        msh1,
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 4>>(&cps0),
        std::get_if<CollisionPolygonSphere<CompressedScenePos, 3>>(&cps0),
        history);
}
