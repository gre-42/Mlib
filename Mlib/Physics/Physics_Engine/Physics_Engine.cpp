#include "Physics_Engine.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Intersection_Info.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Interfaces/IControllable.hpp>
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Concave_Triangles.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Grind_Infos.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Raycast_Intersections.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Movables.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Terrain.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PhysicsEngine::PhysicsEngine(const PhysicsEngineConfig& cfg)
    : rigid_bodies_{ cfg }
    , collision_query_{ *this }
    , collision_direction_{ CollisionDirection::FORWARD }
    , surface_contact_db_{ nullptr }
    , contact_smoke_generator_{ nullptr }
    , particle_renderer_{ nullptr }
    , trail_renderer_{ nullptr }
    , cfg_{ cfg }
{}

PhysicsEngine::~PhysicsEngine() = default;

void PhysicsEngine::collide(
    const StaticWorld& world,
    std::list<Beacon>* beacons,
    bool burn_in,
    size_t oversampling_iteration,
    BaseLog* base_log)
{
    rigid_bodies_.transformed_objects_.remove_if([](const RigidBodyAndIntersectableMeshes& rbtm){
        return (rbtm.rigid_body->mass() != INFINITY);
    });
    {
        std::list<RigidBodyVehicle*> olist;
        for (const auto& o : rigid_bodies_.objects_) {
            if ((o.rigid_body->mass() == INFINITY) || o.rigid_body->is_deactivated_avatar())
            {
                continue;
            }
            o.rigid_body->reset_forces(oversampling_iteration);
            olist.push_back(&o.rigid_body.get());
        }
        for (const auto& co : controllables_) {
            co->notify_reset(burn_in, cfg_);
        }
        for (const auto& efp : external_force_providers_) {
            efp->increment_external_forces(olist, burn_in, cfg_, world);
        }
    }
    std::list<std::unique_ptr<IContactInfo>> contact_infos;
    permanent_contacts_.extend_contact_infos(cfg_, contact_infos);
    std::unordered_map<OrderableFixedArray<CompressedScenePos, 2, 3>, IntersectionSceneAndContact> raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>> concave_t0_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo> grind_infos;
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<ScenePos, 3>>> ridge_intersection_points;
    SatTracker st;
    if (surface_contact_db_ == nullptr) {
        THROW_OR_ABORT("surface_contact_db not set");
    }
    if (contact_smoke_generator_ == nullptr) {
        THROW_OR_ABORT("contact_smoke_generator not set");
    }
    if (trail_renderer_ == nullptr) {
        THROW_OR_ABORT("trail_renderer not set");
    }
    CollisionHistory history{
        .burn_in = burn_in,
        .cfg = cfg_,
        .world = world,
        .st = st,
        .surface_contact_db = *surface_contact_db_,
        .csg = *contact_smoke_generator_,
        .tr = *trail_renderer_,
        .beacons = beacons,
        .contact_infos = contact_infos,
        .raycast_intersections = raycast_intersections,
        .concave_t0_intersections = concave_t0_intersections,
        .grind_infos = grind_infos,
        .ridge_intersection_points = ridge_intersection_points,
        .ridge_map = rigid_bodies_.ridge_map(),
        .base_log = base_log
    };
    for (const auto& o : rigid_bodies_.objects_) {
        if ((o.rigid_body->mass() == INFINITY) || o.rigid_body->is_deactivated_avatar())
        {
            continue;
        }
        if (o.has_meshes()) {
            rigid_bodies_.transform_object_and_add(o);
        }
        o.rigid_body->collide_with_air(history);
    }
    collision_direction_ = (collision_direction_ == CollisionDirection::FORWARD)
        ? CollisionDirection::BACKWARD
        : CollisionDirection::FORWARD;
    collide_with_movables(
        collision_direction_,
        rigid_bodies_,
        history);
    collide_with_terrain(
        rigid_bodies_,
        history);
    // Handling rays before grind_infos so new grind_infos can be created
    // by rays also.
    collide_raycast_intersections(raycast_intersections);
    collide_grind_infos(cfg_, world, contact_infos, grind_infos);
    collide_concave_triangles(cfg_, concave_t0_intersections, ridge_intersection_points);
    solve_contacts(contact_infos, cfg_.dt_substeps());
}

void PhysicsEngine::move_rigid_bodies(
    const StaticWorld& world,
    std::list<Beacon>* beacons,
    const PhysicsPhase& phase)
{
    for (const auto& rbm : rigid_bodies_.objects_) {
        if (rbm.rigid_body->is_deactivated_avatar()) {
            continue;
        }
        auto& rb = rbm.rigid_body;
        assert_true(rb->mass() != INFINITY);
        rb->advance_time(cfg_, world, beacons, phase);
    }
}

void PhysicsEngine::move_particles(const StaticWorld& world)
{
    if (contact_smoke_generator_ == nullptr) {
        THROW_OR_ABORT("contact_smoke_generator not set");
    }
    contact_smoke_generator_->advance_time(cfg_.dt_substeps());
    if (particle_renderer_ != nullptr) {
        particle_renderer_->move(cfg_.dt_substeps(), world);
    }
    if (trail_renderer_ != nullptr) {
        trail_renderer_->move(cfg_.dt_substeps(), world);
    }
}

void PhysicsEngine::move_advance_times(const StaticWorld& world) {
    advance_times_.advance_time(cfg_.dt, world);
}

void PhysicsEngine::burn_in(
    const StaticWorld& world,
    float duration)
{
    for (const auto& o : rigid_bodies_.objects_) {
        if (o.rigid_body->is_deactivated_avatar()) {
            continue;
        }
        for (auto& [_, e] : o.rigid_body->engines_) {
            e.set_surface_power(EnginePowerIntent{
                .state = EngineState::OFF,
                .surface_power = NAN});
        }
    }
    for (float time = 0; time < duration; time += cfg_.dt_substeps()) {
        collide(
            world,
            nullptr,                            // beacons
            true,                               // true = burn_in
            SIZE_MAX,                           // oversampling_iteration
            nullptr);                           // base_log
        if (time < duration / 2) {
            for (const auto& o : rigid_bodies_.objects_) {
                if (o.rigid_body->is_deactivated_avatar()) {
                    continue;
                }
                o.rigid_body->rbp_.w_ = 0;
            }
        }
        move_rigid_bodies(
            world,
            nullptr,  // nullptr=beacons
            PhysicsPhase{
                .burn_in = true,
                .substep = SIZE_MAX
            });
    }
}

void PhysicsEngine::set_surface_contact_db(SurfaceContactDb& surface_contact_db) {
    if (surface_contact_db_ != nullptr) {
        THROW_OR_ABORT("Surface contact DB already set");
    }
    surface_contact_db_ = &surface_contact_db;
}

void PhysicsEngine::set_contact_smoke_generator(ContactSmokeGenerator& contact_smoke_generator) {
    if (contact_smoke_generator_ != nullptr) {
        THROW_OR_ABORT("Contact smoke generator already set");
    }
    contact_smoke_generator_ = &contact_smoke_generator;
}

void PhysicsEngine::set_particle_renderer(IParticleRenderer& particle_renderer) {
    if (particle_renderer_ != nullptr) {
        THROW_OR_ABORT("Particle renderer already set");
    }
    particle_renderer_ = &particle_renderer;
}

void PhysicsEngine::set_trail_renderer(ITrailRenderer& trail_renderer) {
    if (trail_renderer_ != nullptr) {
        THROW_OR_ABORT("Trail renderer already set");
    }
    trail_renderer_ = &trail_renderer;
}

void PhysicsEngine::add_external_force_provider(IExternalForceProvider& efp)
{
    external_force_providers_.push_back(&efp);
}

void PhysicsEngine::add_controllable(IControllable& co)
{
    if (!controllables_.insert(&co).second) {
        THROW_OR_ABORT("IControllable already added");
    }
}

void PhysicsEngine::remove_controllable(IControllable& co) {
    if (controllables_.erase(&co) != 1) {
        THROW_OR_ABORT("IControllable does not exist");
    }
}
