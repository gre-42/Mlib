#include "Physics_Engine.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Collision/Grind_Info.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/Controllable.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Concave_Triangles.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Grind_Infos.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_Raycast_Intersections.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Movables.hpp>
#include <Mlib/Physics/Physics_Engine/Colliders/Collide_With_Terrain.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PhysicsEngine::PhysicsEngine(
    const PhysicsEngineConfig& cfg,
    bool check_objects_deleted_on_destruction)
: rigid_bodies_{cfg},
  collision_query_{*this},
  collision_direction_{CollisionDirection::FORWARD},
  contact_smoke_generator_{nullptr},
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
        bool success = true;
        if (!rigid_bodies_.rigid_bodies_.empty()) {
            success = false;
            lerr() << "~PhysicsEngine: " << rigid_bodies_.rigid_bodies_.size() << " rigid_bodies still exist.";
            for (const auto& [k, v] : rigid_bodies_.rigid_bodies_) {
                lerr() << "  " << k->name();
            }
        }
        if (!rigid_bodies_.objects_.empty()) {
            success = false;
            lerr() << "~PhysicsEngine: " << rigid_bodies_.objects_.size() << " objects still exist.";
            for (const auto& o : rigid_bodies_.objects_) {
                lerr() << "  " << o.rigid_body.name();
            }
        }
        if (!advance_times_.advance_times_to_delete_.empty()) {
            success = false;
            lerr() << "~PhysicsEngine: " << advance_times_.advance_times_to_delete_.size() << " advance_times_to_delete still exist.";
            for (const auto& o : advance_times_.advance_times_to_delete_) {
                lerr() << "  " << typeid(*o).name();
            }
        }
        if (!advance_times_.advance_times_shared_.empty()) {
            success = false;
            lerr() << "~PhysicsEngine: " << advance_times_.advance_times_shared_.size() << " advance_times_shared still exist.";
            for (const auto& o : advance_times_.advance_times_shared_) {
                const auto& od = *o;
                lerr() << "  " << typeid(od).name();
            }
        }
        if (!advance_times_.advance_times_ptr_.empty()) {
            success = false;
            lerr() << "~PhysicsEngine: " << advance_times_.advance_times_ptr_.size() << " advance_times_ptr still exist.";
            for (const auto& o : advance_times_.advance_times_ptr_) {
                lerr() << "  " << typeid(*o).name();
            }
        }
        if (!success) {
            verbose_abort("~PhysicsEngine contains dangling pointers");
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
        return (rbtm.rigid_body.mass() != INFINITY);
    });
    {
        std::list<RigidBodyVehicle*> olist;
        for (const auto& o : rigid_bodies_.objects_) {
            if ((o.rigid_body.mass() == INFINITY) || o.rigid_body.is_deactivated_avatar())
            {
                continue;
            }
            o.rigid_body.reset_forces(oversampling_iteration);
            olist.push_back(&o.rigid_body);
        }
        for (const auto& co : controllables_) {
            co->notify_reset(burn_in, cfg_);
        }
        for (const auto& efp : external_force_providers_) {
            efp->increment_external_forces(olist, burn_in, cfg_);
        }
    }
    std::list<std::unique_ptr<ContactInfo>> contact_infos;
    permanent_contacts_.extend_contact_infos(cfg_, contact_infos);
    for (const auto& o : rigid_bodies_.objects_) {
        if ((o.rigid_body.mass() == INFINITY) || o.rigid_body.is_deactivated_avatar())
        {
            continue;
        }
        if (o.smeshes.empty() && o.dmeshes.empty()) {
            lerr() << "WARNING: Object \"" << o.rigid_body.name() << "\" has no meshes";
        }
        rigid_bodies_.transform_object_and_add(o);
        o.rigid_body.collide_with_air(cfg_, contact_infos);
    }
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact> raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>> concave_t0_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo> grind_infos;
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<double, 3>>> ridge_intersection_points;
    SatTracker st;
    if (contact_smoke_generator_ == nullptr) {
        THROW_OR_ABORT("contact_smoke_generator not set");
    }
    CollisionHistory history{
        .burn_in = burn_in,
        .cfg = cfg_,
        .st = st,
        .scdb = *contact_smoke_generator_,
        .beacons = beacons,
        .contact_infos = contact_infos,
        .raycast_intersections = raycast_intersections,
        .concave_t0_intersections = concave_t0_intersections,
        .grind_infos = grind_infos,
        .ridge_intersection_points = ridge_intersection_points,
        .ridge_map = rigid_bodies_.ridge_map(),
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
    collide_concave_triangles(cfg_, concave_t0_intersections, ridge_intersection_points);
    solve_contacts(contact_infos, cfg_.dt / (float)cfg_.oversampling);
}

void PhysicsEngine::move_rigid_bodies(std::list<Beacon>* beacons) {
    for (const auto& rbm : rigid_bodies_.objects_) {
        if (rbm.rigid_body.is_deactivated_avatar()) {
            continue;
        }
        auto& rb = rbm.rigid_body;
        assert_true(rb.mass() != INFINITY);
        rb.advance_time(cfg_, beacons);
    }
}

void PhysicsEngine::advance_smoke_generator_lifetimes() {
    if (contact_smoke_generator_ == nullptr) {
        THROW_OR_ABORT("contact_smoke_generator not set");
    }
    contact_smoke_generator_->advance_time(cfg_.dt / (float)cfg_.oversampling);
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
        if (o.rigid_body.is_deactivated_avatar()) {
            continue;
        }
        for (auto& [_, e] : o.rigid_body.engines_) {
            e.set_surface_power(EnginePowerIntent{
                .state = EngineState::OFF,
                .surface_power = NAN});
        }
    }
    for (float time = 0; time < duration; time += cfg_.dt / (float)cfg_.oversampling) {
        collide(
            nullptr,        // beacons
            true,           // true = burn_in
            SIZE_MAX,       // oversampling_iteration
            nullptr);       // base_log
        if (time < duration / 2) {
            for (const auto& o : rigid_bodies_.objects_) {
                if (o.rigid_body.is_deactivated_avatar()) {
                    continue;
                }
                o.rigid_body.rbi_.T_ = 0;
                o.rigid_body.rbi_.rbp_.w_ = 0;
            }
        }
        move_rigid_bodies(nullptr);  // nullptr=beacons
    }
}

void PhysicsEngine::set_contact_smoke_generator(ContactSmokeGenerator& contact_smoke_generator) {
    if (contact_smoke_generator_ != nullptr) {
        THROW_OR_ABORT("Contact smoke generator already set");
    }
    contact_smoke_generator_ = &contact_smoke_generator;
}

void PhysicsEngine::add_external_force_provider(ExternalForceProvider& efp)
{
    external_force_providers_.push_back(&efp);
}

void PhysicsEngine::add_controllable(Controllable& co)
{
    if (!controllables_.insert(&co).second) {
        THROW_OR_ABORT("Controllable already added");
    }
}

void PhysicsEngine::remove_controllable(Controllable& co) {
    if (controllables_.erase(&co) != 1) {
        THROW_OR_ABORT("Controllable does not exist");
    }
}
