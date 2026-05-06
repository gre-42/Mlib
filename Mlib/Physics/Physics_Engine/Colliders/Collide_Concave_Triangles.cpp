#include "Collide_Concave_Triangles.hpp"
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Record/Ridge_Intersection_Points_Bvh.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

void Mlib::collide_concave_triangles(
    const PhysicsEngineConfig& cfg,
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>>& concave_t0_intersections,
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<ScenePos, 3>>>& ridge_intersection_points)
{
    for (auto& [rb1, cs] : concave_t0_intersections) {
        auto it = ridge_intersection_points.find(rb1);
        if (it != ridge_intersection_points.end()) {
            RidgeIntersectionPointsBvh bvh{ cfg };
            for (const auto& p : it->second) {
                bvh.insert(p);
            }
            for (const auto& c : cs) {
                if (bvh.has_neighbor(c.iinfo.intersection_point)) {
                    continue;
                }
                handle_line_triangle_intersection(c.scene, c.iinfo);
            }
        } else {
            for (const auto& c : cs) {
                handle_line_triangle_intersection(c.scene, c.iinfo);
            }
        }
    }
}
