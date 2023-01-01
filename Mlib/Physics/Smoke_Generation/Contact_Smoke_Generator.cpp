#include "Contact_Smoke_Generator.hpp"
#include <Mlib/Physics/Collision/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ContactSmokeGenerator::ContactSmokeGenerator(
    SurfaceContactDb& surface_contact_db,
    SmokeParticleGenerator& smoke_particle_generator)
: surface_contact_db_{surface_contact_db},
  smoke_particle_generator_{smoke_particle_generator}
{}

ContactSmokeGenerator::~ContactSmokeGenerator() = default;

void ContactSmokeGenerator::notify_destroyed(Object* destroyed_object) {
    if (tire_smoke_trail_generators_.erase(dynamic_cast<RigidBodyVehicle*>(destroyed_object)) != 1) {
        THROW_OR_ABORT("Could not find surface contact info to be deleted");
    }
}

SurfaceContactInfo* ContactSmokeGenerator::notify_contact(
    const FixedArray<double, 3>& intersection_point,
    const IntersectionScene& c)
{
    if (c.history.burn_in) {
        return nullptr;
    }
    SurfaceContactInfo* surface_contact_info = surface_contact_db_.get_contact_info(intersection_point, c);
    if (surface_contact_info == nullptr) {
        return nullptr;
    }
    if (surface_contact_info->smoke_particle_resource_name.empty()) {
        return surface_contact_info;
    }
    auto v0 = c.o0.rbi_.rbp_.velocity_at_position(intersection_point);
    auto v1 = c.o1.rbi_.rbp_.velocity_at_position(intersection_point);
    auto dvel2 = sum(squared(v0 - v1));
    if (dvel2 < squared(surface_contact_info->minimum_velocity_for_smoke)) {
        return surface_contact_info;
    }
    c.o1.destruction_observers.add(this, ObserverAlreadyExistsBehavior::IGNORE);
    auto& tstg = tire_smoke_trail_generators_[&c.o1];
    auto tstgit = tstg.find(c.tire_id1);
    if (tstgit == tstg.end()) {
        if (!tstg.try_emplace(c.tire_id1,
            smoke_particle_generator_,
            surface_contact_info->smoke_particle_resource_name,
            surface_contact_info->smoke_particle_instance_prefix,
            surface_contact_info->smoke_particle_generation_dt,
            surface_contact_info->smoke_particle_animation_duration).second)
        {
            THROW_OR_ABORT("Could not insert smoke trail generator");
        }
    }
    tstg.at(c.tire_id1).maybe_generate(intersection_point);
    return surface_contact_info;
}

void ContactSmokeGenerator::advance_time(float dt) {
    for (auto& [_, m] : tire_smoke_trail_generators_) {
        for (auto& [_1, g] : m) {
            g.advance_time(dt);
        }
    }
}
