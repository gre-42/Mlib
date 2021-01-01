#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

class RigidBodyIntegrator;
class PhysicsEngine;

class CollisionQuery {
public:
    CollisionQuery(PhysicsEngine& physics_engine);
    bool can_see(
        const FixedArray<float, 3>& watcher,
        const FixedArray<float, 3>& watched,
        const RigidBodyIntegrator* excluded0 = nullptr,
        const RigidBodyIntegrator* excluded1 = nullptr,
        bool only_terrain = false);
    bool can_see(
        const RigidBodyIntegrator& watcher,
        const RigidBodyIntegrator& watched,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0);
    bool can_see(
        const RigidBodyIntegrator& watcher,
        const FixedArray<float, 3>& watched,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0);
private:
    PhysicsEngine& physics_engine_;
};

}
