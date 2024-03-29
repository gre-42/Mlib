#pragma once
#include <string>

namespace Mlib {

enum class ParticleSubstrate {
    AIR,
    SKIDMARK
};

ParticleSubstrate particle_substrate_from_string(const std::string& s);

}
