#include "Terrain_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TerrainType Mlib::terrain_type_from_string(const std::string& tt) {
    if (tt == "grass") {
        return TerrainType::GRASS;
    } else if (tt == "wayside1_grass") {
        return TerrainType::WAYSIDE1_GRASS;
    } else if (tt == "wayside2_grass") {
        return TerrainType::WAYSIDE2_GRASS;
    } else if (tt == "flowers") {
        return TerrainType::FLOWERS;
    } else if (tt == "trees") {
        return TerrainType::TREES;
    } else if (tt == "stone") {
        return TerrainType::STONE;
    } else if (tt == "asphalt") {
        return TerrainType::ASPHALT;
    } else if (tt == "elevated_grass") {
        return TerrainType::ELEVATED_GRASS;
    } else if (tt == "water_floor") {
        return TerrainType::WATER_FLOOR;
    } else if (tt == "water_floor_base") {
        return TerrainType::WATER_FLOOR_BASE;
    } else if (tt == "street_hole") {
        return TerrainType::STREET_HOLE;
    } else if (tt == "building_hole") {
        return TerrainType::BUILDING_HOLE;
    } else if (tt == "ocean_ground") {
        return TerrainType::OCEAN_GROUND;
    } else if (tt == "undefined") {
        return TerrainType::UNDEFINED;
    } else {
        THROW_OR_ABORT("Unknown terrain type: " + tt);
    }
}

std::string Mlib::terrain_type_to_string(TerrainType tt) {
    if (tt == TerrainType::GRASS) {
        return "grass";
    } else if (tt == TerrainType::WAYSIDE1_GRASS) {
        return "wayside1_grass";
    } else if (tt == TerrainType::WAYSIDE2_GRASS) {
        return "wayside2_grass";
    } else if (tt == TerrainType::FLOWERS) {
        return "flowers";
    } else if (tt == TerrainType::TREES) {
        return "trees";
    } else if (tt == TerrainType::STONE) {
        return "stone";
    } else if (tt == TerrainType::ASPHALT) {
        return "asphalt";
    } else if (tt == TerrainType::ELEVATED_GRASS) {
        return "elevated_grass";
    } else if (tt == TerrainType::ELEVATED_GRASS_BASE) {
        return "elevated_grass_base";
    } else if (tt == TerrainType::WATER_FLOOR) {
        return "water_floor";
    } else if (tt == TerrainType::WATER_FLOOR_BASE) {
        return "water_floor_base";
    } else if (tt == TerrainType::STREET_HOLE) {
        return "street_hole";
    } else if (tt == TerrainType::BUILDING_HOLE) {
        return "building_hole";
    } else if (tt == TerrainType::OCEAN_GROUND) {
        return "ocean_ground";
    } else if (tt == TerrainType::UNDEFINED) {
        return "undefined";
    } else {
        THROW_OR_ABORT("Unknown terrain type: " + std::to_string(int(tt)));
    }
}

std::string Mlib::to_string(TerrainType tt) {
    return terrain_type_to_string(tt);
}