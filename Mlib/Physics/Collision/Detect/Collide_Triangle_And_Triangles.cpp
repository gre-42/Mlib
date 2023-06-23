#include "Collide_Triangle_And_Triangles.hpp"
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>

using namespace Mlib;

void Mlib::collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionTriangleSphere& t0,
    const CollisionHistory& history)
{
    // Mesh-sphere <-> triangle-sphere intersection
    if (!msh1.mesh->intersects(t0.bounding_sphere)) {
        return;
    }
    // Mesh-sphere <-> triangle-plane intersection
    if (!msh1.mesh->intersects(t0.plane)) {
        return;
    }
    for (const auto& r1 : msh1.mesh->get_ridges_sphere()) {
        if (!r1.bounding_sphere.intersects(t0.bounding_sphere)) {
            continue;
        }
        if (!r1.bounding_sphere.intersects(t0.plane)) {
            continue;
        }
        handle_line_triangle_intersection(IntersectionScene{
            .o0 = o0,
            .o1 = o1,
            .mesh0 = msh0,
            .mesh1 = msh1.mesh.get(),
            .l1 = nullptr,
            .r1 = &r1,
            .t0 = t0,
            .tire_id1 = SIZE_MAX,
            .mesh0_material = t0.physics_material,
            .mesh1_material = msh1.physics_material,
            .l1_is_normal = false,
            .default_collision_type = CollisionType::REFLECT,
            .history = history});
    }
}
