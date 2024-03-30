#include "Contact_Smoke_Generator.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
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
    : surface_contact_db_{ surface_contact_db }
    , smoke_particle_generator_{ smoke_particle_generator }
{}

ContactSmokeGenerator::~ContactSmokeGenerator() {
    if (!tire_smoke_trail_generators_.empty()) {
        verbose_abort("Tire smoke generators remain during shutdown");
    }
}

void ContactSmokeGenerator::notify_destroyed(const RigidBodyVehicle& destroyed_object) {
    if (tire_smoke_trail_generators_.erase(const_cast<RigidBodyVehicle*>(&destroyed_object)) != 1) {
        THROW_OR_ABORT("Could not find surface contact info to be deleted");
    }
}

SurfaceContactInfo* ContactSmokeGenerator::notify_contact(
    const FixedArray<double, 3>& intersection_point,
    const FixedArray<float, 3>& rotation,
    const FixedArray<double, 3>& surface_normal,
    const IntersectionScene& c)
{
    if (c.history.burn_in) {
        return nullptr;
    }
    SurfaceContactInfo* surface_contact_info = surface_contact_db_.get_contact_info(c);
    if (surface_contact_info == nullptr) {
        return nullptr;
    }
    if (surface_contact_info->smoke_infos.empty()) {
        return surface_contact_info;
    }
    auto v0 = c.o0.rbp_.velocity_at_position(intersection_point);
    auto v1_a = c.o1.rbp_.velocity_at_position(intersection_point);
    auto v1_s =
        c.o1.get_velocity_at_tire_contact(surface_normal.casted<float>(), c.tire_id1) +
        c.o1.get_abs_tire_z(c.tire_id1) *
        (
            c.o1.get_tire_angular_velocity(c.tire_id1) *
            c.o1.get_tire_radius(c.tire_id1));
    auto dvel_a = std::sqrt(sum(squared(v0 - v1_a)));
    auto dvel_s = std::sqrt(sum(squared(v0 - v1_s)));
    for (const auto& [i, smoke_info] : enumerate(surface_contact_info->smoke_infos)) {
        const auto& af = smoke_info.vehicle_velocity_to_smoke_particle_frequency;
        const auto& sf = smoke_info.tire_velocity_to_smoke_particle_frequency;
        auto f =
            (af.empty() ? 0.f : af(dvel_a)) +
            (sf.empty() ? 0.f : sf(dvel_s));
        if (f < 1e-12f) {
            continue;
        }
        auto& tstg = tire_smoke_trail_generators_[&c.o1];
        if (tstg.empty()) {
            c.o1.destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION }, ObserverAlreadyExistsBehavior::IGNORE);
        }
        std::pair<size_t, size_t> key{ c.tire_id1, i };
        if (auto tstgit = tstg.find(key); tstgit == tstg.end()) {
            if (!tstg.try_emplace(key, smoke_particle_generator_).second)
            {
                THROW_OR_ABORT("Could not insert smoke trail generator");
            }
        }
        tstg.at(key).maybe_generate(
            intersection_point,
            rotation,
            smoke_info.smoke_particle_resource_name,
            smoke_info.smoke_particle_instance_prefix,
            smoke_info.smoke_particle_animation_duration,
            1.f / f,
            ParticleType::INSTANCE);
    }
    return surface_contact_info;
}

void ContactSmokeGenerator::advance_time(float dt) {
    for (auto& [_, m] : tire_smoke_trail_generators_) {
        for (auto& [_1, g] : m) {
            g.advance_time(dt);
        }
    }
}
