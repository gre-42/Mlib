#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct TerrainWayPoints;
struct Way;

std::list<TerrainWayPoints> get_terrain_way_points(const std::map<std::string, Way>& ways);

}
