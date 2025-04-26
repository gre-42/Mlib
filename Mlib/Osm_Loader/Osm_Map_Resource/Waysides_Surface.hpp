#pragma once
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct WaysideResourceNamesSurface{
    double tangential_distance;
    double normal_distance;
    double gradient_dx;
    double max_gradient;
    std::vector<ParsedResourceName> resource_names;
};

void from_json(const nlohmann::json& j, WaysideResourceNamesSurface& w);

}
