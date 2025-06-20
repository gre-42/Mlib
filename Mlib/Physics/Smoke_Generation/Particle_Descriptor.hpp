#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

enum class ParticleType;

struct ParticleDescriptor {
    VariableAndHash<std::string> resource_name;
    float air_resistance;
    float animation_duration;
    ParticleType type;
};

void from_json(const nlohmann::json& j, ParticleDescriptor& item);

}
