#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <memory>
#include <string>

namespace Mlib {

struct ParticleDescriptor;
template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
enum class ParticleType;
template <class T>
class VariableAndHash;

class ParticleTrailGenerator {
public:
    explicit ParticleTrailGenerator(SmokeParticleGenerator& smoke_generator);
    ~ParticleTrailGenerator();
    void advance_time(float dt);
    void maybe_generate(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        const ParticleDescriptor& trail,
        float particle_generation_dt,
        const std::string& instance_prefix,
        ParticleType particle_type);
private:
    SmokeParticleGenerator& smoke_generator_;
    float trail_lifetime_;
};

}
