#include "Particle_Substrate.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

ParticleSubstrate Mlib::particle_substrate_from_string(const std::string& s) {
    static std::map<std::string, ParticleSubstrate> m{
        {"air", ParticleSubstrate::AIR},
        {"skidmark", ParticleSubstrate::SKIDMARK}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown particle substrate: \"" + s + '"');
    }
    return it->second;
}
