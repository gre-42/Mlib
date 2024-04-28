#include "Get_Terrain_Way_Points.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Way_Points.hpp>

using namespace Mlib;

std::list<TerrainWayPoints> Mlib::get_terrain_way_points(const std::map<std::string, Way>& ways)
{
    std::list<TerrainWayPoints> result;
    for (const auto& [_, way] : ways) {
        const auto& tags = way.tags;
        auto wit = tags.find("way_points");
        if (wit == tags.end()) {
            continue;
        }
        auto cit = tags.find("way_points_class");
        result.push_back(TerrainWayPoints{
            .way = way,
            .orientation = way_points_orientation_from_string(wit->second),
            .class_ = cit == tags.end()
                ? WayPointsClass::GROUND
                : way_points_class_from_string(cit->second)
        });
    }
    return result;
}
