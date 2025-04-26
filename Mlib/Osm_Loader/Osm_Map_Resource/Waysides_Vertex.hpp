#pragma once
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct WaysideResourceNamesVertex {
    float min_dist;
    float max_dist;
    std::vector<ParsedResourceName> resource_names;
};

void from_json(const nlohmann::json& j, WaysideResourceNamesVertex& w);

}
