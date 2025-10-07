#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

enum class DamageSource {
    NONE = 0,
    CRASH = 1 << 1,
    BULLET = 1 << 2,
    ANY = CRASH | BULLET
};

inline bool any(DamageSource a) {
    return (a != DamageSource::NONE);
}

inline DamageSource operator & (DamageSource a, DamageSource b) {
    return (DamageSource)((int)a & (int)b);
}

inline DamageSource& operator |= (DamageSource& a, DamageSource b) {
    (int&)a |= (int)b;
    return a;
}

void from_json(const nlohmann::json& j, DamageSource& damage_source);

}
