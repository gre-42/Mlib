#include "Get_Buildings_Or_Wall_Barriers.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

std::list<Building> Mlib::get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top)
{
    std::list<Building> result;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("layer") &&
            (safe_stoi(tags.at("layer")) != 0)) {
            continue;
        }
        if (tags.find("building") != tags.end()) {
            if ((building_type != BuildingType::BUILDING) || excluded_buildings.contains(tags.at("building"))) {
                continue;
            }
        } else if (tags.find("barrier") != tags.end()) {
            if ((building_type != BuildingType::WALL_BARRIER) || !included_barriers.contains(tags.at("barrier"))) {
                continue;
            }
        } else if (tags.find("spawn_line") != tags.end()) {
            if ((building_type != BuildingType::SPAWN_LINE) || (tags.at("spawn_line") != "yes")) {
                continue;
            }
        } else if (tags.find("way_points") != tags.end()) {
            if ((building_type != BuildingType::WAYPOINTS) || (tags.at("way_points") != "yes")) {
                continue;
            }
        } else {
            continue;
        }
        float building_top = default_building_top;
        building_top = parse_meters(tags, "height", building_top);
        building_top = parse_meters(tags, "building:height", building_top);
        auto vs = tags.find("vertical_subdivision");
        if (vs == tags.end()) {
            result.push_back(Building{
                .id = w.first,
                .way = w.second,
                .levels = {BuildingLevel{
                    .top = building_top,
                    .bottom = building_bottom
                }}});
        } else if (vs->second == "socle") {
            float socle_height = 1.5;
            if (building_top <= socle_height) {
                throw std::runtime_error("Building height too small for socle");
            }
            result.push_back(Building{
                .id = w.first,
                .way = w.second,
                .levels = {
                    BuildingLevel{
                        .top = socle_height,
                        .bottom = building_bottom,
                        .extra_width = 0.5f},
                    BuildingLevel{
                        .top = building_top,
                        .bottom = socle_height}
                }});
        } else {
            throw std::runtime_error("Unknown vertical_subdivision: " + vs->second);
        }
    }
    return result;
}
