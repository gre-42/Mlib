#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <list>
#include <set>

namespace Mlib {

class ExternalForceProvider;
class Controllable;
struct Beacon;
class BaseLog;
enum class CollisionDirection;
class ContactSmokeGenerator;
class ITrailRenderer;

class PhysicsEngine {
    friend CollisionQuery;
public:
    PhysicsEngine(
        const PhysicsEngineConfig& cfg,
        bool check_objects_deleted_on_destruction = true);
    ~PhysicsEngine();
    void add_external_force_provider(ExternalForceProvider& efp);
    void add_controllable(Controllable& co);
    void remove_controllable(Controllable& co);
    void collide(
        std::list<Beacon>* beacons,
        bool burn_in,
        size_t oversampling_iteration,
        BaseLog* base_log);
    void move_rigid_bodies(std::list<Beacon>* beacons);
    void advance_smoke_generator_lifetimes();
    void move_advance_times();
    void burn_in(float duration);
    void set_contact_smoke_generator(ContactSmokeGenerator& contact_smoke_generator);
    void set_trail_renderer(ITrailRenderer& trail_renderer);

    RigidBodies rigid_bodies_;
    AdvanceTimes advance_times_;
    CollisionQuery collision_query_;
    PermanentContacts permanent_contacts_;
private:
    CollisionDirection collision_direction_;
    ContactSmokeGenerator* contact_smoke_generator_;
    ITrailRenderer* trail_renderer_;
    std::list<ExternalForceProvider*> external_force_providers_;
    std::set<Controllable*> controllables_;
    PhysicsEngineConfig cfg_;
    bool check_objects_deleted_on_destruction_;
};

}
