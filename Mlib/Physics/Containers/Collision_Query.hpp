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
        const RigidBodyIntegrator* excluded1 = nullptr);
    bool can_see(
        const RigidBodyIntegrator& watcher,
        const RigidBodyIntegrator& watched);
private:
    PhysicsEngine& physics_engine_;
};

}
