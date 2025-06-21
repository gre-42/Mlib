#include "Particle_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

ParticleType Mlib::particle_type_from_string(const std::string& s) {
    static std::map<std::string, ParticleType> m{
        {"none", ParticleType::NONE},
        {"smoke", ParticleType::SMOKE},
        {"skidmark", ParticleType::SKIDMARK},
        {"water_wave", ParticleType::WATER_WAVE},
        {"sea_spray", ParticleType::SEA_SPRAY},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown particle type: \"" + s + '"');
    }
    return it->second;
}

std::string Mlib::particle_type_to_string(ParticleType s) {
    switch (s) {
    case ParticleType::NONE:
        return "none";
    case ParticleType::SMOKE:
        return "smoke";
    case ParticleType::SKIDMARK:
        return "skidmark";
    case ParticleType::WATER_WAVE:
        return "water_wave";
    case ParticleType::SEA_SPRAY:
        return "sea_spray";
    }
    THROW_OR_ABORT("Unknown particle type: " + std::to_string((int)s));
}
