#pragma once
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

enum class TerrainType {
    GRASS,
    WAYSIDE1_GRASS,
    WAYSIDE2_GRASS,
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
    OCEAN_GROUND,
    UNDEFINED
};

TerrainType terrain_type_from_string(const std::string& tt);

std::string terrain_type_to_string(TerrainType tt);

std::string to_string(TerrainType tt);

void from_json(const nlohmann::json& j, TerrainType& e);

}
