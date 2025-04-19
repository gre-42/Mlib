#include "Terrain_Type.hpp"
#include <Mlib/Json/Base.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TerrainType Mlib::terrain_type_from_string(const std::string& s) {
    static std::map<std::string, TerrainType> m{
        {"grass", TerrainType::GRASS },
        {"wayside1_grass", TerrainType::WAYSIDE1_GRASS },
        {"wayside2_grass", TerrainType::WAYSIDE2_GRASS },
        {"flowers", TerrainType::FLOWERS },
        {"trees", TerrainType::TREES },
        {"stone", TerrainType::STONE },
        {"asphalt", TerrainType::ASPHALT },
        {"elevated_grass", TerrainType::ELEVATED_GRASS },
        {"elevated_grass_base", TerrainType::ELEVATED_GRASS_BASE },
        {"water_floor", TerrainType::WATER_FLOOR },
        {"water_floor_base", TerrainType::WATER_FLOOR_BASE },
        {"street_hole", TerrainType::STREET_HOLE },
        {"building_hole", TerrainType::BUILDING_HOLE },
        {"ocean_ground", TerrainType::OCEAN_GROUND },
        {"undefined", TerrainType::UNDEFINED } };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown terrain type: \"" + s + '"');
    }
    return it->second;
}

std::string Mlib::terrain_type_to_string(TerrainType tt) {
    switch (tt) {
        case TerrainType::GRASS: return "grass";
        case TerrainType::WAYSIDE1_GRASS: return "wayside1_grass";
        case TerrainType::WAYSIDE2_GRASS: return "wayside2_grass";
        case TerrainType::FLOWERS: return "flowers";
        case TerrainType::TREES: return "trees";
        case TerrainType::STONE: return "stone";
        case TerrainType::ASPHALT: return "asphalt";
        case TerrainType::ELEVATED_GRASS: return "elevated_grass";
        case TerrainType::ELEVATED_GRASS_BASE: return "elevated_grass_base";
        case TerrainType::WATER_FLOOR: return "water_floor";
        case TerrainType::WATER_FLOOR_BASE: return "water_floor_base";
        case TerrainType::STREET_HOLE: return "street_hole";
        case TerrainType::BUILDING_HOLE: return "building_hole";
        case TerrainType::OCEAN_GROUND: return "ocean_ground";
        case TerrainType::UNDEFINED: return "undefined";
    }
    THROW_OR_ABORT("Unknown terrain type: " + std::to_string(int(tt)));
}

std::string Mlib::to_string(TerrainType tt) {
    return terrain_type_to_string(tt);
}

void Mlib::from_json(const nlohmann::json& j, TerrainType& t) {
    t = terrain_type_from_string(j.get<std::string>());
}
