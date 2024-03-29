#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
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
class ITrailExtender;
class IDynamicLight;
class DynamicLights;
struct BulletProperties;

class Bullet:
    public DestructionObserver<const IPlayer&>,
    public DestructionObserver<const ITeam&>,
    public CollisionObserver,
    public IAdvanceTime,
    public DanglingBaseClass
{
public:
    Bullet(
        Scene& scene,
        SmokeParticleGenerator& smoke_generator,
        AdvanceTimes& advance_times,
        RigidBodyVehicle& rigid_body,
        RigidBodies& rigid_bodies,
        IPlayer* gunner,
        ITeam* team,
        std::string bullet_node_name,
        const BulletProperties& props,
        std::unique_ptr<ITrailExtender> trace_extender,
        DynamicLights& dynamic_lights,
        DeleteNodeMutex& delete_node_mutex,
        std::chrono::steady_clock::time_point time);
    ~Bullet();
    virtual void notify_destroyed(const IPlayer& destroyed_object) override;
    virtual void notify_destroyed(const ITeam& destroyed_object) override;
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) override;
    virtual void notify_collided(
        const FixedArray<double, 3>& intersection_point,
        std::chrono::steady_clock::time_point time,
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) override;
private:
    void cause_damage(
        const FixedArray<double, 3>& intersection_point,
        RigidBodyVehicle& rigid_body);
    void cause_damage(
        RigidBodyVehicle& rigid_body,
        float amount);
    void notify_kill(RigidBodyVehicle& rigid_body_vehicle);
    Scene& scene_;
    SmokeParticleGenerator& smoke_generator_;
    AdvanceTimes& advance_times_;
    RigidBodyPulses& rigid_body_pulses_;
    RigidBodies& rigid_bodies_;
    IPlayer* gunner_;
    ITeam* team_;
    std::string bullet_node_name_;
    const BulletProperties& props_;
    float lifetime_;
    SmokeTrailGenerator trail_generator_;
    bool has_trail_;
    std::unique_ptr<ITrailExtender> trace_extender_;
    std::unique_ptr<IDynamicLight> light_before_impact_;
    std::unique_ptr<IDynamicLight> light_after_impact_;
    DynamicLights& dynamic_lights_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
