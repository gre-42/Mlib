#include "Collide_Triangle_And_Triangles.hpp"
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
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
    const CollisionPolygonSphere<4>* q0,
    const CollisionPolygonSphere<3>* t0,
    const CollisionHistory& history)
{
    auto collide = [&](const auto& poly0){
        // Mesh-sphere <-> triangle-sphere intersection
        if (!msh1.mesh->intersects(poly0.bounding_sphere)) {
            return;
        }
        // Mesh-sphere <-> triangle-plane intersection
        if (!msh1.mesh->intersects(poly0.polygon.plane())) {
            return;
        }
        for (const auto& r1 : msh1.mesh->get_ridges_sphere()) {
            if (!r1.bounding_sphere.intersects(poly0.bounding_sphere)) {
                continue;
            }
            handle_line_triangle_intersection(IntersectionScene{
                .o0 = o0,
                .o1 = o1,
                .mesh0 = msh0,
                .mesh1 = msh1.mesh.get(),
                .l1 = nullptr,
                .r1 = &r1,
                .q0 = q0,
                .t0 = t0,
                .tire_id1 = SIZE_MAX,
                .mesh0_material = poly0.physics_material,
                .mesh1_material = msh1.physics_material,
                .l1_is_normal = false,
                .default_collision_type = CollisionType::REFLECT,
                .history = history});
        }
    };
    if (q0 != nullptr) {
        collide(*q0);
    }
    if (t0 != nullptr) {
        collide(*t0);
    }
}
