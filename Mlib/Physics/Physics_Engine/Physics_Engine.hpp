#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Permanent_Contacts.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <chrono>
#include <list>
#include <set>

namespace Mlib {

class IExternalForceProvider;
class IControllable;
struct Beacon;
class BaseLog;
enum class CollisionDirection;
class SurfaceContactDb;
class ContactSmokeGenerator;
class ITrailRenderer;
struct StaticWorld;
struct PhysicsPhase;

class PhysicsEngine {
public:
    explicit PhysicsEngine(const PhysicsEngineConfig& cfg);
    ~PhysicsEngine();
    void add_external_force_provider(IExternalForceProvider& efp);
    void remove_external_force_provider(IExternalForceProvider& efp);
    void add_controllable(IControllable& co);
    void remove_controllable(IControllable& co);
    void collide(
        const StaticWorld& world,
        std::list<Beacon>* beacons,
        bool burn_in,
        size_t oversampling_iteration,
        BaseLog* base_log);
    void move_rigid_bodies(
        const StaticWorld& world,
        std::list<Beacon>* beacons,
        const PhysicsPhase& phase);
    void move_particles(const StaticWorld& world);
    void move_advance_times(const StaticWorld& world);
    void burn_in(
        const StaticWorld& world,
        float duration);
    void set_surface_contact_db(SurfaceContactDb& surface_contact_db);
    void set_contact_smoke_generator(ContactSmokeGenerator& contact_smoke_generator);
    void set_trail_renderer(ITrailRenderer& trail_renderer);
    inline const PhysicsEngineConfig& config() const { return cfg_; }

    RigidBodies rigid_bodies_;
    AdvanceTimes advance_times_;
    CollisionQuery collision_query_;
    PermanentContacts permanent_contacts_;
private:
    CollisionDirection collision_direction_;
    SurfaceContactDb* surface_contact_db_;
    ContactSmokeGenerator* contact_smoke_generator_;
    ITrailRenderer* trail_renderer_;
    std::list<IExternalForceProvider*> external_force_providers_;
    std::set<IControllable*> controllables_;
    PhysicsEngineConfig cfg_;
};

}
