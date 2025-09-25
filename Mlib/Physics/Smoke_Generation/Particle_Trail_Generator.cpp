#include "Particle_Trail_Generator.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Descriptor.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

ParticleTrailGenerator::ParticleTrailGenerator(SmokeParticleGenerator& smoke_generator)
    : smoke_generator_{ smoke_generator }
    , yangle_rng_{ 121, 0.f, (float)(2. * M_PI) }
{}

ParticleTrailGenerator::~ParticleTrailGenerator() = default;

void ParticleTrailGenerator::generate(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    const FixedArray<float, 3>& velocity,
    float texture_layer,
    const ParticleDescriptor& trail,
    const std::string& instance_prefix,
    ParticleContainer particle_container,
    const StaticWorld& static_world)
{
    auto r = [&]() {
        switch (trail.rotation) {
        case ParticleRotation::EMITTER:
            return rotation;
        case ParticleRotation::RANDOM_YANGLE:
            return FixedArray<float, 3>{0.f, yangle_rng_(), 0.f};
        }
        THROW_OR_ABORT("Unknown particle rotation");
    }();
    smoke_generator_.generate_root(
        trail.resource_name,
        VariableAndHash<std::string>{instance_prefix + smoke_generator_.generate_suffix()},
        position,
        r,
        velocity,
        trail.air_resistance,
        texture_layer,
        trail.animation_duration,
        particle_container,
        static_world);
}
