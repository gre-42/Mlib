#pragma once
#include <string>

namespace Mlib {

enum class ParticleType {
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
    return (ParticleType)((int)a & (int)b);
}

inline ParticleType operator | (ParticleType a, ParticleType b) {
    return (ParticleType)((int)a | (int)b);
}

inline ParticleType& operator &= (ParticleType& a, ParticleType b) {
    (int&)a &= (int)b;
    return a;
}

inline ParticleType& operator |= (ParticleType& a, ParticleType b) {
    (int&)a |= (int)b;
    return a;
}

inline ParticleType operator ~ (ParticleType t) {
    return (ParticleType)(~(int)t);
}

}
