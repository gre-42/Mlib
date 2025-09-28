#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

enum class ParticleRotation {
    EMITTER,
    RANDOM_YANGLE
};

ParticleRotation particle_rotation_from_string(const std::string& s);

enum class ParticleType;

struct ParticleDescriptor {
    VariableAndHash<std::string> resource_name;
    float air_resistance_halflife;
    float animation_duration;
    ParticleRotation rotation;
    ParticleType type;
};

void from_json(const nlohmann::json& j, ParticleDescriptor& item);

}
