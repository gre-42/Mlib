#include "Physics_Engine.hpp"
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Physics/Collision/Collision_History.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Controllable.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Grind_Infos.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Raycast_Intersections.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Movables.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Terrain.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

PhysicsEngine::PhysicsEngine(
    const PhysicsEngineConfig& cfg,
    bool check_objects_deleted_on_destruction)
: rigid_bodies_{cfg},
  collision_query_{*this},
  collision_direction_{CollisionDirection::FORWARD},
  cfg_{cfg},
  check_objects_deleted_on_destruction_{check_objects_deleted_on_destruction}
{}

PhysicsEngine::~PhysicsEngine() {
    // The physics thread calls "delete_scheduled_advance_times".
    // However, scene destruction (which schedules deletion) happens after physics thread joining
    // and before PhysicsEngine destruction.
    // => We need to call "delete_scheduled_advance_times" in the PhysicsEngine destructor (in the main thread).
    // No special handling is required for objects (i.e. rigid bodies), because their deletion is not scheduled,
    // but happens instantaneously.
    advance_times_.delete_scheduled_advance_times();
    if (check_objects_deleted_on_destruction_) {
        if (!rigid_bodies_.objects_.empty()) {
            std::cerr << "~PhysicsEngine: " << rigid_bodies_.objects_.size() << " objects still exist." << std::endl;
            for (const auto& o : rigid_bodies_.objects_) {
                std::cerr << "  " << o.rigid_body->name() << std::endl;
            }
        }
        if (!advance_times_.advance_times_to_delete_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_to_delete_.size() << " advance_times_to_delete still exist." << std::endl;
            for (const auto& o : advance_times_.advance_times_to_delete_) {
                std::cerr << "  " << typeid(*o).name() << std::endl;
            }
        }
        if (!advance_times_.advance_times_shared_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_shared_.size() << " advance_times_shared still exist." << std::endl;
            for (const auto& o : advance_times_.advance_times_shared_) {
                const auto& od = *o;
                std::cerr << "  " << typeid(od).name() << std::endl;
            }
        }
        if (!advance_times_.advance_times_ptr_.empty()) {
            std::cerr << "~PhysicsEngine: " << advance_times_.advance_times_ptr_.size() << " advance_times_ptr still exist." << std::endl;
            for (const auto& o : advance_times_.advance_times_ptr_) {
                std::cerr << "  " << typeid(*o).name() << std::endl;
            }
        }
    }
}

void PhysicsEngine::collide(
    std::list<Beacon>* beacons,
    bool burn_in,
    size_t oversampling_iteration,
    BaseLog* base_log)
{
    rigid_bodies_.transformed_objects_.remove_if([](const RigidBodyAndIntersectableMeshes& rbtm){
        return (rbtm.rigid_body->mass() != INFINITY);
    });
    {
        std::list<std::shared_ptr<RigidBodyVehicle>> olist;
        for (const auto& o : rigid_bodies_.objects_) {
            if (o.rigid_body->mass() != INFINITY) {
                o.rigid_body->reset_forces(oversampling_iteration);
                olist.push_back(o.rigid_body);
            }
        }
        for (const auto& co : controllables_) {
            co->notify_reset(burn_in, cfg_);
        }
        for (const auto& efp : external_force_providers_) {
            efp->increment_external_forces(olist, burn_in, cfg_);
        }
    }
    std::list<std::unique_ptr<ContactInfo>> contact_infos;
    for (const auto& o : rigid_bodies_.objects_) {
        if (o.rigid_body->mass() != INFINITY) {
            if (o.smeshes.empty() && o.dmeshes.empty()) {
                std::cerr << "WARNING: Object has no meshes" << std::endl;
            }
            rigid_bodies_.transform_object_and_add(o);
            o.rigid_body->collide_with_air(cfg_, contact_infos);
        }
    }
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact> raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo> grind_infos;
    SatTracker st;
    CollisionHistory history{
        .cfg = cfg_,
        .st = st,
        .beacons = beacons,
        .contact_infos = contact_infos,
        .raycast_intersections = raycast_intersections,
        .grind_infos = grind_infos,
        .base_log = base_log
    };
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
    collide_grind_infos(cfg_, contact_infos, grind_infos);
    solve_contacts(contact_infos, cfg_.dt / cfg_.oversampling);
}

void PhysicsEngine::move_rigid_bodies(std::list<Beacon>* beacons) {
    for (const auto& rbm : rigid_bodies_.objects_) {
        auto& rb = rbm.rigid_body;
        assert_true(rb->mass() != INFINITY);
        rb->advance_time(cfg_, beacons);
    }
}

void PhysicsEngine::move_advance_times() {
    for (const auto& a : advance_times_.advance_times_shared_) {
        if (!advance_times_.advance_times_to_delete_.contains(a.get())) {
            a->advance_time(cfg_.dt);
        }
    }
    for (const auto& a : advance_times_.advance_times_ptr_) {
        if (!advance_times_.advance_times_to_delete_.contains(a)) {
            a->advance_time(cfg_.dt);
        }
    }
}

void PhysicsEngine::burn_in(float duration) {
    for (const auto& o : rigid_bodies_.objects_) {
        for (auto& [_, e] : o.rigid_body->engines_) {
            e.set_surface_power(EnginePowerIntent{.surface_power = NAN});
        }
    }
    for (float time = 0; time < duration; time += cfg_.dt / cfg_.oversampling) {
        collide(
            nullptr,        // beacons
            true,           // true = burn_in
            SIZE_MAX,       // oversampling_iteration
            nullptr);       // base_log
        if (time < duration / 2) {
            for (const auto& o : rigid_bodies_.objects_) {
                o.rigid_body->rbi_.T_ = 0;
                o.rigid_body->rbi_.rbp_.w_ = 0;
            }
        }
        move_rigid_bodies(nullptr);  // nullptr=beacons
    }
}

void PhysicsEngine::add_external_force_provider(ExternalForceProvider* efp)
{
    external_force_providers_.push_back(efp);
}

void PhysicsEngine::add_controllable(Controllable* co)
{
    if (!controllables_.insert(co).second) {
        throw std::runtime_error("Controllable already added");
    }
}

void PhysicsEngine::remove_controllable(Controllable* co) {
    if (controllables_.erase(co) != 1) {
        throw std::runtime_error("Controllable does not exist");
    }
}
