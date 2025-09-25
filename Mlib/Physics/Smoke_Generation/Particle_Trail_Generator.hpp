#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <cstddef>
#include <memory>
#include <string>

namespace Mlib {

struct ParticleDescriptor;
template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
enum class ParticleContainer;
template <class T>
class VariableAndHash;
struct StaticWorld;

class ParticleTrailGenerator {
public:
    explicit ParticleTrailGenerator(SmokeParticleGenerator& smoke_generator);
    ~ParticleTrailGenerator();
    void generate(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        const FixedArray<float, 3>& velocity,
        float texture_layer,
        const ParticleDescriptor& trail,
        const std::string& instance_prefix,
        ParticleContainer particle_container,
        const StaticWorld& static_world);
private:
    SmokeParticleGenerator& smoke_generator_;
    FastUniformRandomNumberGenerator<float> yangle_rng_;
};

}
