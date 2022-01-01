#include "Get_Terrain_Way_Points.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Way_Points.hpp>

using namespace Mlib;

std::list<TerrainWayPoints> Mlib::get_terrain_way_points(const std::map<std::string, Way>& ways)
{
    std::list<TerrainWayPoints> result;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        auto it = tags.find("way_points");
        if (it == tags.end()) {
            continue;
        }
        result.push_back(TerrainWayPoints{
            .way = w.second,
            .orientation = way_point_orientation_from_string(it->second)
        });
    }
    return result;
}
