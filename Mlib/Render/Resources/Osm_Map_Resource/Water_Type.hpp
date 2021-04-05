#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class WaterType {
    UNDEFINED,
    HOLE
};

inline WaterType water_type_from_string(const std::string& wt) {
    if (wt == "undefined") {
        return WaterType::UNDEFINED;
    } else if (wt == "hole") {
        return WaterType::HOLE;
    } else {
        throw std::runtime_error("Unknown water type");
    }
}

inline std::string water_type_to_string(WaterType wt) {
    if (wt == WaterType::UNDEFINED) {
        return "undefined";
    } else if (wt == WaterType::HOLE) {
        return "hole";
    } else {
        throw std::runtime_error("Unknown water type");
    }
}

inline std::string to_string(WaterType wt) {
    return water_type_to_string(wt);
}

}
