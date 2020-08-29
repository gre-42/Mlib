#pragma once

namespace Mlib {

class RigidBodyIntegrator;
class PhysicsEngine;

class CollisionQuery {
public:
    CollisionQuery(PhysicsEngine& physics_engine);
    bool can_see(const RigidBodyIntegrator& watcher, const RigidBodyIntegrator& watched);
private:
    PhysicsEngine& physics_engine_;
};

}
