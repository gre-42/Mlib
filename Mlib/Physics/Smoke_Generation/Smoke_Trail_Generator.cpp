#include "Smoke_Trail_Generator.hpp"
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

SmokeTrailGenerator::SmokeTrailGenerator(SmokeParticleGenerator& smoke_generator)
    : smoke_generator_{ smoke_generator }
    , trail_lifetime_{ 0.f }
{}

SmokeTrailGenerator::~SmokeTrailGenerator() = default;

void SmokeTrailGenerator::advance_time(float dt) {
    trail_lifetime_ += dt;
}

void SmokeTrailGenerator::maybe_generate(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    float air_resistance,
    const VariableAndHash<std::string>& resource_name,
    const std::string& instance_prefix,
    float animation_duration,
    float particle_generation_dt,
    ParticleType particle_type)
{
    if (trail_lifetime_ > particle_generation_dt) {
        trail_lifetime_ = 0.f;
        smoke_generator_.generate_root(
            resource_name,
            VariableAndHash<std::string>{instance_prefix + smoke_generator_.generate_suffix()},
            position,
            rotation,
            velocity,
            air_resistance,
            animation_duration,
            particle_type);
    }
}
