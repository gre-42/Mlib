#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Building;
struct Way;
enum class BuildingType;

std::list<Building> get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top);

}
