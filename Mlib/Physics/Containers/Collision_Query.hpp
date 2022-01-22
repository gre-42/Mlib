#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

class RigidBodyVehicle;
class PhysicsEngine;

class CollisionQuery {
public:
    CollisionQuery(PhysicsEngine& physics_engine);
    bool can_see(
        const FixedArray<float, 3>& watcher,
        const FixedArray<float, 3>& watched,
        const RigidBodyVehicle* excluded0 = nullptr,
        const RigidBodyVehicle* excluded1 = nullptr,
        bool only_terrain = false,
        FixedArray<float, 3>* intersection_point = nullptr,
        FixedArray<float, 3>* intersection_normal = nullptr,
        const RigidBodyVehicle** seen_object = nullptr);
    bool can_see(
        const RigidBodyVehicle& watcher,
        const RigidBodyVehicle& watched,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0,
        FixedArray<float, 3>* intersection_point = nullptr,
        FixedArray<float, 3>* intersection_normal = nullptr,
        const RigidBodyVehicle** seen_object = nullptr);
    bool can_see(
        const RigidBodyVehicle& watcher,
        const FixedArray<float, 3>& watched,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0,
        FixedArray<float, 3>* intersection_point = nullptr,
        FixedArray<float, 3>* intersection_normal = nullptr,
        const RigidBodyVehicle** seen_object = nullptr);
private:
    PhysicsEngine& physics_engine_;
};

}
