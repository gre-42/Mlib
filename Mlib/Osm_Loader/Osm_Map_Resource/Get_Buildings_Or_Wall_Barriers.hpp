#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct Building;
struct Way;
class FacadeTextureCycle;

enum class BuildingType {
    BUILDING,
    WALL_BARRIER,
    SPAWN_LINE
};

std::list<Building> get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top,
    const std::vector<std::string>& socle_textures,
    FacadeTextureCycle& ftc);

}
