#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
enum class ParticleType;
template <class T>
class VariableAndHash;

class SmokeTrailGenerator {
public:
    explicit SmokeTrailGenerator(SmokeParticleGenerator& smoke_generator);
    ~SmokeTrailGenerator();
    void advance_time(float dt);
    void maybe_generate(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float air_resistance,
        const VariableAndHash<std::string>& resource_name,
        const std::string& instance_prefix,
        float animation_duration,
        float particle_generation_dt,
        ParticleType particle_type);
private:
    SmokeParticleGenerator& smoke_generator_;
    float trail_lifetime_;
};

}
