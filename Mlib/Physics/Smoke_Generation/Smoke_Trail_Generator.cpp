#include "Smoke_Trail_Generator.hpp"
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>

using namespace Mlib;

SmokeTrailGenerator::SmokeTrailGenerator(
    SmokeParticleGenerator& smoke_generator,
    std::string resource_name,
    std::string instance_prefix,
    float animation_duration)
: smoke_generator_{smoke_generator},
  resource_name_{std::move(resource_name)},
  instance_prefix_{std::move(instance_prefix)},
  animation_duration_{animation_duration},
  trail_lifetime_{0.f}
{}

SmokeTrailGenerator::~SmokeTrailGenerator() = default;

void SmokeTrailGenerator::advance_time(float dt) {
    trail_lifetime_ += dt;
}

void SmokeTrailGenerator::maybe_generate(
    const FixedArray<double, 3>& position,
    float particle_generation_dt)
{
    if (trail_lifetime_ > particle_generation_dt) {
        trail_lifetime_ = 0.f;
        smoke_generator_.generate_root(
            resource_name_,
            instance_prefix_ + smoke_generator_.generate_suffix(),
            position,
            animation_duration_);
    }
}
