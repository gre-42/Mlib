#pragma once
#include <string>

namespace Mlib {

enum class ParticleType {
    SMOKE,
    SKIDMARK,
    WATER_WAVE,
    SEA_SPRAY
};

ParticleType particle_type_from_string(const std::string& s);
std::string particle_type_to_string(ParticleType s);

}
