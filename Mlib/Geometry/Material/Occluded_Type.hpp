#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class OccludedType {
    OFF,
    LIGHT_MAP_COLOR,
    LIGHT_MAP_DEPTH
};

inline OccludedType occluded_type_from_string(const std::string& str) {
    if (str == "off") {
        return OccludedType::OFF;
    } else if (str == "color") {
        return OccludedType::LIGHT_MAP_COLOR;
    } else if (str == "depth") {
        return OccludedType::LIGHT_MAP_DEPTH;
    }
    throw std::runtime_error("Unknown occluded type");
}

}
