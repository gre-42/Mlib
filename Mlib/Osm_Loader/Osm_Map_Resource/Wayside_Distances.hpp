#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct WaysideDistances{
    double tangential_distance;
    double normal_distance;
    double gradient_dx;
    double max_gradient;
};

void from_json(const nlohmann::json& j, WaysideDistances& item);

}
