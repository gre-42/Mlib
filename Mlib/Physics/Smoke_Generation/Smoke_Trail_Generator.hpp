#pragma once
#include <cstddef>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
enum class ParticleType;

class SmokeTrailGenerator {
public:
    explicit SmokeTrailGenerator(SmokeParticleGenerator& smoke_generator);
    ~SmokeTrailGenerator();
    void advance_time(float dt);
    void maybe_generate(
        const FixedArray<double, 3>& position,
        std::string resource_name,
        std::string instance_prefix,
        float animation_duration,
        float particle_generation_dt,
        ParticleType particle_type);
private:
    SmokeParticleGenerator& smoke_generator_;
    float trail_lifetime_;
};

}
