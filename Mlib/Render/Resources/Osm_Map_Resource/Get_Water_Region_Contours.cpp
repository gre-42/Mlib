#include "Get_Water_Region_Contours.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Water_Type.hpp>

using namespace Mlib;

std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>> Mlib::get_water_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>> result;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        WaterType terrain_type;
        if (auto it = tags.find("water_region"); (it != tags.end()) && (it->second == "hole")) {
            terrain_type = WaterType::HOLE;
        } else {
            continue;
        }
        if (w.second.nd.empty()) {
            continue;
        }
        if (w.second.nd.front() != w.second.nd.back()) {
            throw std::runtime_error("Region is not closed: " + w.first);
        }
        result.push_back({terrain_type, {}});
        auto it = w.second.nd.begin();
        ++it;
        for (; it != w.second.nd.end(); ++it) {
            auto p = nodes.at(*it).position;
            result.back().second.push_back({p(0), p(1), 0.f});
        }
        if (compute_area_clockwise(w.second.nd, nodes, 1.f) > 0.f) {
            result.back().second.reverse();
        }
    }
    return result;
}
