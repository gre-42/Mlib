#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <string>

namespace Mlib {

class RigidBody;
struct RigidBodyIntegrator;
class Scene;
class SceneNode;
class AdvanceTimes;

class Bullet: public DestructionObserver, public CollisionObserver, public AdvanceTime {
public:
    Bullet(
        Scene& scene,
        SceneNode& bullet_node,
        AdvanceTimes& advance_times,
        RigidBody& rigid_body,
        const std::string& bullet_node_name,
        float max_lifetime,
        float damage);
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    virtual void notify_collided(
        RigidBody& rigid_body,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) override;
private:
    Scene& scene_;
    AdvanceTimes& advance_times_;
    RigidBodyIntegrator& rigid_body_integrator_;
    std::string bullet_node_name_;
    float max_lifetime_;
    float lifetime_;
    float damage_;
};

}
