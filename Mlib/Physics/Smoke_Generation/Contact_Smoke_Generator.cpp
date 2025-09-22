#include "Contact_Smoke_Generator.hpp"
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Audio/Audio_Periodicity.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ContactSmokeGenerator::ContactSmokeGenerator(
    OneShotAudio& one_shot_audio,
    SmokeParticleGenerator& air_smoke_particle_generator,
    SmokeParticleGenerator& skidmark_smoke_particle_generator,
    SmokeParticleGenerator& sea_wave_smoke_particle_generator)
    : one_shot_audio_{ one_shot_audio }
    , air_smoke_particle_generator_{ air_smoke_particle_generator }
    , skidmark_smoke_particle_generator_{ skidmark_smoke_particle_generator }
    , sea_wave_smoke_particle_generator_{ sea_wave_smoke_particle_generator }
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
    if (c.history.phase.burn_in) {
        return;
    }
    if (c.surface_contact_info == nullptr) {
        return;
    }
    if (c.surface_contact_info->emission.empty()) {
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
    for (const auto& smoke_info : c.surface_contact_info->emission) {
        const auto& af = smoke_info.vehicle_velocity.smoke_particle_frequency;
        const auto& sf = smoke_info.tire_velocity.smoke_particle_frequency;
        auto f =
            (af.empty() ? 0.f : af(dvel_a)) +
            (sf.empty() ? 0.f : sf(dvel_s));
        if (f < 1e-12f) {
            continue;
        }
        auto& tstg = [&]() -> ContactSmokeAndAudio& {
            auto it = tire_smoke_trail_generators_.find(&c.o1);
            if (it == tire_smoke_trail_generators_.end()) {
                auto res = tire_smoke_trail_generators_.try_emplace(&c.o1);
                if (!res.second) {
                    verbose_abort("ContactSmokeGenerator tire_smoke_trail_generators_.try_emplace error");
                }
                c.o1.destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION }, ObserverAlreadyExistsBehavior::IGNORE);
                return res.first->second;
            }
            return it->second;
        }();
        auto ce_generate = [&](ContactEmissions& ce){
            if (auto n = ce.maybe_generate(1.f / f); n != 0) {
                if (smoke_info.audio != nullptr) {
                    smoke_info.audio->play(
                        one_shot_audio_,
                        AudioPeriodicity::APERIODIC,
                        {intersection_point, c.o1.rbp_.velocity_at_position(intersection_point)});
                }
                if (smoke_info.visual.has_value()) {
                    assert_true(ce.particle_trail_generator.has_value());
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
                    auto ptg_generate = [&](const FixedArray<ScenePos, 3>& p){
                        ce.particle_trail_generator->generate(
                            p,
                            rotation,
                            dirx * pvel,
                            smoke_info.visual->particle,
                            smoke_info.visual->smoke_particle_instance_prefix,
                            ParticleContainer::INSTANCE,
                            c.history.world);
                    };
                    if (!ce.old_position.has_value() || (n > c.history.cfg.max_interpolated_particles)) {
                        ptg_generate(intersection_point);
                    } else {
                        for (uint32_t i = 1; i <= n; ++i) {
                            ptg_generate(lerp(*ce.old_position, intersection_point, integral_to_float<ScenePos>(i) / n));
                        }
                    }
                    ce.old_position.emplace(intersection_point);
                }
            }
        };
        auto create_particle_trail_generator = [&](ContactEmissions& ce){
            if (smoke_info.visual.has_value()) {
                auto& pgen = [&]() -> SmokeParticleGenerator& {
                    switch (smoke_info.visual->particle.type) {
                        case ParticleType::NONE: THROW_OR_ABORT("Particle type \"none\" does not require a contact smoke generator");
                        case ParticleType::SMOKE: return air_smoke_particle_generator_;
                        case ParticleType::SKIDMARK: return skidmark_smoke_particle_generator_;
                        case ParticleType::WATER_WAVE: THROW_OR_ABORT("Water waves do not require a contact smoke generator");
                        case ParticleType::SEA_SPRAY: return sea_wave_smoke_particle_generator_;
                    };
                    THROW_OR_ABORT("Unknown particle type");
                }();
                ce.particle_trail_generator.emplace(pgen);
            }
        };
        auto generate = [&](const auto& key, auto& map){
            if (auto tstgit = map.find(key); tstgit == map.end()) {
                auto ceit = map.try_emplace(key);
                if (!ceit.second) {
                    verbose_abort("Could not insert smoke trail generator");
                }
                ContactEmissions& ce = ceit.first->second;
                create_particle_trail_generator(ce);
                ce_generate(ce);
            } else {
                ce_generate(tstgit->second);
            }
        };
        switch (smoke_info.affinity) {
        case SurfaceSmokeAffinity::PAIR: {
                std::pair<size_t, const SurfaceSmokeInfo*> key{ c.tire_id1, &smoke_info };
                generate(key, tstg.smoke);
                continue;
            }
        case SurfaceSmokeAffinity::TIRE: {
                size_t key = c.tire_id1;
                generate(key, tstg.audio);
                continue;
            }
        };
        THROW_OR_ABORT("Unknown surface from affinity");
    }
}

void ContactSmokeGenerator::advance_time(float dt) {
    for (auto& [_, m] : tire_smoke_trail_generators_) {
        for (auto& [_1, g] : m.smoke) {
            g.maybe_generate.advance_time(dt);
        }
        for (auto& [_1, g] : m.audio) {
            g.maybe_generate.advance_time(dt);
        }
    }
}
