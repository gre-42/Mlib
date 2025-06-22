#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct BoundaryLimitation {
    float max = 0.3f;
    float falloff = 0.8f;
};

void from_json(const nlohmann::json& j, BoundaryLimitation& l);

}
