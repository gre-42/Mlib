#include "Get_Terrain_Region_Contours.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

std::list<std::pair<TerrainType, std::list<FixedArray<double, 3>>>> Mlib::get_terrain_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    std::list<std::pair<TerrainType, std::list<FixedArray<double, 3>>>> result;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("layer") &&
            (safe_stoi(tags.at("layer")) != 0)) {
            continue;
        }
        TerrainType terrain_type;
        // if (auto it = tags.find("leisure"); (it != tags.end()) && (it->second == "park")) {
        //     terrain_type = TerrainType::GRASS;
        // } else if (auto it = tags.find("landuse"); it != tags.end() && (it->second == "grass" || it->second == "meadow")) {
        //     terrain_type = TerrainType::GRASS;
        // } else {
        //     continue;
        // }
        if (auto it = tags.find("terrain_region"); it != tags.end()) {
            terrain_type = terrain_type_from_string(it->second);
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
