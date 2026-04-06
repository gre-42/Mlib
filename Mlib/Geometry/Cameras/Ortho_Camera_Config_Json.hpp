#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct OrthoCameraConfig;

void from_json(const nlohmann::json& j, OrthoCameraConfig& config);

}
