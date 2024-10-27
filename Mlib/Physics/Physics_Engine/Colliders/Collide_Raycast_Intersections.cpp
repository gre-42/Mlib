#include "Collide_Raycast_Intersections.hpp"
#include <Mlib/Physics/Collision/Record/Handle_Line_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>

void Mlib::collide_raycast_intersections(
    const std::unordered_map<const FixedArray<ScenePos, 2, 3>*, IntersectionSceneAndContact>& raycast_intersections)
{
    for (const auto& [l1, cc] : raycast_intersections) {
        handle_line_triangle_intersection(cc.scene, cc.iinfo);
    }
}
