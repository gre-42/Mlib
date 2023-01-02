#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Trail_Generator.hpp>
#include <mutex>
#include <string>

namespace Mlib {

class RigidBodies;
class RigidBodyVehicle;
class RigidBodyPulses;
class Scene;
class SceneNode;
class AdvanceTimes;
class SceneNodeResources;
class DeleteNodeMutex;
class IPlayer;
class ITeam;
class SmokeParticleGenerator;

class Bullet: public DestructionObserver, public CollisionObserver, public AdvanceTime {
public:
    Bullet(
        Scene& scene,
        SmokeParticleGenerator& smoke_generator,
        AdvanceTimes& advance_times,
        RigidBodyVehicle& rigid_body,
        RigidBodies& rigid_bodies,
        IPlayer* gunner,
        ITeam* team,
        const std::string& bullet_node_name,
        const std::string& bullet_explosion_resource_name,
        float bullet_explosion_animation_time,
        float max_lifetime,
        float damage,
        float damage_radius,
        const std::string& trail_resource,
        float trail_dt,
        float trail_animation_time,
        DeleteNodeMutex& delete_node_mutex);
    ~Bullet();
    virtual void notify_destroyed(Object& obj) override;
    virtual void advance_time(float dt) override;
    virtual void notify_collided(
        const FixedArray<double, 3>& intersection_point,
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) override;
private:
    void cause_damage(
        const FixedArray<double, 3>& intersection_point,
        RigidBodyVehicle& rigid_body);
    void notify_kill(RigidBodyVehicle& rigid_body_vehicle);
    Scene& scene_;
    SmokeParticleGenerator& smoke_generator_;
    AdvanceTimes& advance_times_;
    RigidBodyPulses& rigid_body_pulses_;
    RigidBodies& rigid_bodies_;
    IPlayer* gunner_;
    ITeam* team_;
    std::string bullet_node_name_;
    std::string bullet_explosion_resource_name_;
    float bullet_explosion_animation_time_;
    float max_lifetime_;
    float lifetime_;
    float damage_;
    float damage_radius_;
    SmokeTrailGenerator trail_generator_;
    bool has_trail_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
