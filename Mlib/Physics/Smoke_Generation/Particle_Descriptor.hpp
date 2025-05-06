#pragma once
#include <Mlib/Json/Base.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

struct ParticleDescriptor {
    VariableAndHash<std::string> resource_name;
    float air_resistance;
    float animation_duration;
};

void from_json(const nlohmann::json& j, ParticleDescriptor& item);

}
