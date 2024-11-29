#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionSceneAndContact;
class RigidBodyVehicle;
struct PhysicsEngineConfig;

void collide_concave_triangles(
    const PhysicsEngineConfig& cfg,
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>>& concave_t0_intersections,
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<ScenePos, 3>>>& ridge_intersection_points_bvh);

}
