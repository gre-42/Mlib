#pragma once
#include <Mlib/Physics/Smoke_Generation/Particle_Descriptor.hpp>

namespace Mlib {

struct ConstantParticleTrail {
    ParticleDescriptor particle;
    float generation_dt;
};

void from_json(const nlohmann::json& j, ConstantParticleTrail& item);

}
