#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class ParticleType: uint32_t {
    NONE = 0,
    SMOKE = 1 << 0,
    SKIDMARK = 1 << 1,
    WATER_WAVE = 1 << 2,
    SEA_SPRAY = 1 << 3
};

ParticleType particle_type_from_string(const std::string& s);
std::string particle_type_to_string(ParticleType s);

inline bool any(ParticleType t) {
    return t != ParticleType::NONE;
}

inline ParticleType operator & (ParticleType a, ParticleType b) {
    return (ParticleType)((uint32_t)a & (uint32_t)b);
}

inline ParticleType operator | (ParticleType a, ParticleType b) {
    return (ParticleType)((uint32_t)a | (uint32_t)b);
}

inline ParticleType& operator &= (ParticleType& a, ParticleType b) {
    (uint32_t&)a &= (uint32_t)b;
    return a;
}

inline ParticleType& operator |= (ParticleType& a, ParticleType b) {
    (uint32_t&)a |= (uint32_t)b;
    return a;
}

}
