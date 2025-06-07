#pragma once
#include <string>

namespace Mlib {

enum class WaterType {
    UNDEFINED,
    STEEP_HOLE,
    SHALLOW_HOLE,
    SHALLOW_LAKE
};

WaterType water_type_from_string(const std::string& wt);

std::string water_type_to_string(WaterType wt);

std::string to_string(WaterType wt);

}
