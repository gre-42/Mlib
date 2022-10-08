#pragma once
#include <string>

namespace Mlib {

enum class TerrainType {
    GRASS,
    WAYSIDE_GRASS,
    FLOWERS,
    TREES,
    STONE,
    ASPHALT,
    ELEVATED_GRASS,
    ELEVATED_GRASS_BASE,
    WATER_FLOOR,
    WATER_FLOOR_BASE,
    STREET_HOLE,
    BUILDING_HOLE,
    UNDEFINED
};

TerrainType terrain_type_from_string(const std::string& tt);

std::string terrain_type_to_string(TerrainType tt);

std::string to_string(TerrainType tt);

}
