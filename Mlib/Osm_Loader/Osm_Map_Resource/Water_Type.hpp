#pragma once
#include <string>

namespace Mlib {

enum class WaterType {
    NONE = 0,
    UNDEFINED = 1 << 0,
    STEEP_HOLE = 1 << 1,
    SHALLOW_HOLE = 1 << 2,
    SHALLOW_LAKE = 1 << 3,
    ANY_SHALLOW = SHALLOW_HOLE | SHALLOW_LAKE
};

inline bool any(WaterType t) {
    return t != WaterType::NONE;
}

inline WaterType operator & (WaterType a, WaterType b) {
    return (WaterType)((int)a & (int)b);
}

WaterType water_type_from_string(const std::string& wt);

std::string water_type_to_string(WaterType wt);

std::string to_string(WaterType wt);

}
