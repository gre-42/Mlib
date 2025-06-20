#include "Particle_Trail_Generator.hpp"
#include <Mlib/Physics/Smoke_Generation/Particle_Descriptor.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

ParticleTrailGenerator::ParticleTrailGenerator(SmokeParticleGenerator& smoke_generator)
    : smoke_generator_{ smoke_generator }
    , trail_lifetime_{ 0.f }
{}

ParticleTrailGenerator::~ParticleTrailGenerator() = default;

void ParticleTrailGenerator::advance_time(float dt) {
    trail_lifetime_ += dt;
}

void ParticleTrailGenerator::maybe_generate(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    const ParticleDescriptor& trail,
    float particle_generation_dt,
    const std::string& instance_prefix,
    ParticleContainer particle_container)
{
    if (trail_lifetime_ > particle_generation_dt) {
        trail_lifetime_ = 0.f;
        smoke_generator_.generate_root(
            trail.resource_name,
            VariableAndHash<std::string>{instance_prefix + smoke_generator_.generate_suffix()},
            position,
            rotation,
            velocity,
            trail.air_resistance,
            trail.animation_duration,
            particle_container);
    }
}
