#include "Contact_Smoke_Generator.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Substrate.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ContactSmokeGenerator::ContactSmokeGenerator(
    SmokeParticleGenerator& air_smoke_particle_generator,
    SmokeParticleGenerator& skidmark_smoke_particle_generator)
    : air_smoke_particle_generator_{ air_smoke_particle_generator }
    , skidmark_smoke_particle_generator_{ skidmark_smoke_particle_generator }
{}

ContactSmokeGenerator::~ContactSmokeGenerator() {
    if (!tire_smoke_trail_generators_.empty()) {
        verbose_abort("Tire smoke generators remain during shutdown");
    }
}

void ContactSmokeGenerator::notify_destroyed(const RigidBodyVehicle& destroyed_object) {
    if (tire_smoke_trail_generators_.erase(const_cast<RigidBodyVehicle*>(&destroyed_object)) != 1) {
        verbose_abort("Could not find surface contact info to be deleted");
    }
}

void ContactSmokeGenerator::notify_contact(
    const FixedArray<ScenePos, 3>& intersection_point,
    const FixedArray<float, 3>& rotation,
    const FixedArray<SceneDir, 3>& surface_normal,
    const IntersectionScene& c)
{
    if (c.history.burn_in) {
        return;
    }
    if (c.surface_contact_info == nullptr) {
        return;
    }
    if (c.surface_contact_info->smoke_infos.empty()) {
        return;
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
    for (const auto& [i, smoke_info] : enumerate(c.surface_contact_info->smoke_infos)) {
        const auto& af = smoke_info.vehicle_velocity.smoke_particle_frequency;
        const auto& sf = smoke_info.tire_velocity.smoke_particle_frequency;
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
            auto& pgen = [&]() -> SmokeParticleGenerator& {
                switch (smoke_info.particle.substrate) {
                    case ParticleSubstrate::AIR: return air_smoke_particle_generator_;
                    case ParticleSubstrate::SKIDMARK: return skidmark_smoke_particle_generator_;
                };
                THROW_OR_ABORT("Unknoen particle substrate");
            }();
            if (!tstg.try_emplace(key, pgen).second)
            {
                THROW_OR_ABORT("Could not insert smoke trail generator");
            }
        }
        const auto& av = smoke_info.vehicle_velocity.smoke_particle_velocity;
        const auto& sv = smoke_info.tire_velocity.smoke_particle_velocity;
        auto pvel =
            (av.empty() ? 0.f : av(dvel_a)) +
            (sv.empty() ? 0.f : sv(dvel_s));
        auto dirx = c.o1.rbp_.rotation_.column(0);
        if (dot0d(dirx, (intersection_point - c.o1.rbp_.abs_position()).casted<float>()) < 0.f) {
            dirx = -dirx;
        }
        dirx -= surface_normal.casted<float>() * dot0d(surface_normal.casted<float>(), dirx);
        tstg.at(key).maybe_generate(
            intersection_point,
            rotation,
            dirx * pvel,
            smoke_info.particle,
            1.f / f,
            smoke_info.smoke_particle_instance_prefix,
            ParticleType::INSTANCE);
    }
}

void ContactSmokeGenerator::advance_time(float dt) {
    for (auto& [_, m] : tire_smoke_trail_generators_) {
        for (auto& [_1, g] : m) {
            g.advance_time(dt);
        }
    }
}
