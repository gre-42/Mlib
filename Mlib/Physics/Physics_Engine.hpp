#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <list>
#include <set>
#include <unordered_map>

namespace Mlib {

class ExternalForceProvider;
class Controllable;
struct Beacon;
class ContactInfo;
class BaseLog;
struct IntersectionSceneAndContact;
class SatTracker;
struct GrindInfo;

class PhysicsEngine {
    friend CollisionQuery;
public:
    PhysicsEngine(
        const PhysicsEngineConfig& cfg,
        bool check_objects_deleted_on_destruction = true);
    ~PhysicsEngine();
    void add_external_force_provider(ExternalForceProvider* efp);
    void add_controllable(Controllable* co);
    void remove_controllable(Controllable* co);
    void collide(
        std::list<Beacon>* beacons,
        bool burn_in,
        size_t oversampling_iteration,
        BaseLog* base_log);
    void move_rigid_bodies(std::list<Beacon>* beacons);
    void move_advance_times();
    void burn_in(float duration);

    RigidBodies rigid_bodies_;
    AdvanceTimes advance_times_;
    CollisionQuery collision_query_;
private:
    void collide_with_movables(
        const SatTracker& st,
        std::list<Beacon>* beacons,
        std::list<std::unique_ptr<ContactInfo>>& contact_infos,
        std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
        std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
        BaseLog* base_log);
    void collide_with_terrain(
        const SatTracker& st,
        std::list<Beacon>* beacons,
        std::list<std::unique_ptr<ContactInfo>>& contact_infos,
        std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections,
        std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos,
        BaseLog* base_log);
    void collide_raycast_intersections(
        const std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections);
    void collide_grind_infos(
        std::list<std::unique_ptr<ContactInfo>>& contact_infos,
        const std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos);
    std::list<ExternalForceProvider*> external_force_providers_;
    std::set<Controllable*> controllables_;
    PhysicsEngineConfig cfg_;
    bool check_objects_deleted_on_destruction_;
    bool collide_forward_ = false;
};

}
