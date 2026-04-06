#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Proctree {

struct Properties;
void from_json(const nlohmann::json& j, Properties& properties);

}
