#pragma once
#include <cstddef>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;

class SmokeTrailGenerator {
public:
    SmokeTrailGenerator(
        SmokeParticleGenerator& smoke_generator,
        std::string resource_name,
        std::string instance_prefix,
        float particle_generation_dt,
        float animation_duration);
    ~SmokeTrailGenerator();
    void advance_time(float dt);
    void maybe_generate(const FixedArray<double, 3>& position);
private:
    SmokeParticleGenerator& smoke_generator_;
    std::string resource_name_;
    std::string instance_prefix_;
    float particle_generation_dt_;
    float animation_duration_;
    float trail_lifetime_;
};

}
