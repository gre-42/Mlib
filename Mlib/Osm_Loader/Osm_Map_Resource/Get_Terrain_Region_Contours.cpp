#include "Get_Terrain_Region_Contours.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Region_With_Margin.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::list<RegionWithMargin<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>> Mlib::get_terrain_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    std::list<RegionWithMargin<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>> result;
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
            THROW_OR_ABORT("Region is not closed: " + w.first);
        }
        auto& contour = result.emplace_back(terrain_type, TerrainType::UNDEFINED, (CompressedScenePos)0.f);
        auto it = w.second.nd.begin();
        ++it;
        for (; it != w.second.nd.end(); ++it) {
            auto p = nodes.at(*it).position;
            contour.geometry.push_back(p);
        }
        if (compute_area_clockwise(w.second.nd, nodes, 1.f) > 0.f) {
            contour.geometry.reverse();
        }
    }
    return result;
}
