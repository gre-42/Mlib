#include "Get_Way_Points.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Way_Points.hpp>

using namespace Mlib;

std::list<WayPoints> Mlib::get_way_points(const std::map<std::string, Way>& ways)
{
    std::list<WayPoints> result;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        auto it = tags.find("way_points");
        if (it == tags.end()) {
            continue;
        }
        result.push_back(WayPoints{
            .way = w.second,
            .orientation = way_point_orientation_from_string(it->second)
        });
    }
    return result;
}
