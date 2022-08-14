#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <mutex>
#include <string>

namespace Mlib {

class RigidBodies;
class RigidBodyVehicle;
struct RigidBodyPulses;
class Scene;
class SceneNode;
class AdvanceTimes;
class SceneNodeResources;
class DeleteNodeMutex;

class Bullet: public DestructionObserver, public CollisionObserver, public AdvanceTime {
public:
    Bullet(
        Scene& scene,
        SceneNodeResources& scene_node_resources,
        SceneNode& bullet_node,
        AdvanceTimes& advance_times,
        RigidBodyVehicle& rigid_body,
        RigidBodies& rigid_bodies,
        const std::string& bullet_node_name,
        const std::string& bullet_explosion_resource_name,
        float bullet_explosion_animation_time,
        float max_lifetime,
        float damage,
        float damage_radius,
        DeleteNodeMutex& delete_node_mutex);
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    virtual void notify_collided(
        const FixedArray<double, 3>& intersection_point,
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) override;
private:
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    AdvanceTimes& advance_times_;
    RigidBodyPulses& rigid_body_pulses_;
    RigidBodies& rigid_bodies_;
    std::string bullet_node_name_;
    std::string bullet_explosion_resource_name_;
    float bullet_explosion_animation_time_;
    float max_lifetime_;
    float lifetime_;
    float damage_;
    float damage_radius_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
