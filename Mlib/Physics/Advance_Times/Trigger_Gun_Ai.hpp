#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>

namespace Mlib {

class Gun;
class PhysicsEngine;
class SceneNode;
class RigidBodyIntegrator;

class TriggerGunAi: public DestructionObserver, public AdvanceTime {
public:
    TriggerGunAi(
        SceneNode& base_shooter_node,
        SceneNode& base_target_node,
        SceneNode& gun_node,
        RigidBodyIntegrator& rbi_shooter,
        RigidBodyIntegrator& rbi_target,
        PhysicsEngine& physics_engine,
        Gun& gun);
    virtual void notify_destroyed(void* destroyed_object) override;
    virtual void advance_time(float dt) override;
private:
    SceneNode& base_shooter_node_;
    SceneNode& base_target_node_;
    SceneNode& gun_node_;
    RigidBodyIntegrator& rbi_shooter_;
    RigidBodyIntegrator& rbi_target_;
    PhysicsEngine& physics_engine_;
    Gun& gun_;
};

}
