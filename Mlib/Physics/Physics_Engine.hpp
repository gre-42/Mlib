#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <list>

namespace Mlib {

class ExternalForceProvider;

class PhysicsEngine {
    friend CollisionQuery;
public:
    PhysicsEngine(
        const PhysicsEngineConfig& cfg,
        bool check_objects_deleted_on_destruction = true);
    ~PhysicsEngine();
    void add_external_force_provider(ExternalForceProvider* efp);
    void collide(std::list<FixedArray<float, 3>>& beacons, bool burn_in);
    void move_rigid_bodies(std::list<FixedArray<float, 3>>& beacons);
    void move_advance_times();
    void burn_in(float seconds);

    RigidBodies rigid_bodies_;
    AdvanceTimes advance_times_;
    CollisionQuery collision_query_;
private:
    std::list<ExternalForceProvider*> external_force_providers_;
    PhysicsEngineConfig cfg_;
    bool check_objects_deleted_on_destruction_;
    bool collide_forward_ = false;
};

}
