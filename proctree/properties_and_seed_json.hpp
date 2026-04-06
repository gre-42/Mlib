#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Proctree {

struct PropertiesAndSeed;
void from_json(const nlohmann::json& j, PropertiesAndSeed& ps);

}
