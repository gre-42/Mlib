#pragma once

namespace Mlib {

enum class TerrainType {
    GRASS,
    STONE,
    HOLE,
    UNDEFINED
};

inline std::string terrain_type_to_string(TerrainType tt) {
    if (tt == TerrainType::GRASS) {
        return "grass";
    } else if (tt == TerrainType::STONE) {
        return "stone";
    } else if (tt == TerrainType::HOLE) {
        return "hole";
    } else if (tt == TerrainType::UNDEFINED) {
        return "undefined";
    } else {
        throw std::runtime_error("Unknown terrain type");
    }
}

inline std::string to_string(TerrainType tt) {
    return terrain_type_to_string(tt);
}

}
