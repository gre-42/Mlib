#include "Get_Buildings_Or_Wall_Barriers.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

std::list<Building> Mlib::get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top,
    float uv_scale_facade,
    const std::vector<std::string>& socle_textures,
    FacadeTextureCycle& ftc)
{
    size_t bid = 0;
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
        } else {
            continue;
        }
        std::string style;
        switch (building_type) {
            case BuildingType::BUILDING:
                style = tags.contains("building:material") ? tags.get("building:material") : "";
                break;
            case BuildingType::WALL_BARRIER:
                style = tags.contains("style") ? tags.get("style") : "";
                break;
            case BuildingType::SPAWN_LINE:
                break;
            default:
                throw std::runtime_error("Unknown building type");
        }
        float building_top = default_building_top;
        building_top = parse_meters(tags, "height", building_top);
        building_top = parse_meters(tags, "building:height", building_top);

        FacadeTextureDescriptor middle_ftd;
        if (building_type == BuildingType::BUILDING) {
            if (!style.empty()) {
                auto ft = ftc(style);
                if (ft == nullptr) {
                    // throw std::runtime_error("Unknown building material: \"" + bu.style + '"');
                    std::cerr << "Unknown building material: \"" + style + '"' << std::endl;
                    middle_ftd = ftc(building_top).descriptor;
                } else {
                    middle_ftd = ft->descriptor;
                }
            } else {
                if (ftc.empty()) {
                    throw std::runtime_error("Facade textures empty");
                }
                middle_ftd = ftc(building_top).descriptor;
            }
        }

        auto vs = tags.find("vertical_subdivision");
        bool has_socle =
            ((vs == tags.end()) && (building_type == BuildingType::BUILDING)) ||
            ((vs != tags.end()) && (vs->second == "socle"));
        float socle_height = 1.2;
        if (tags.contains("snap_height", "yes")) {
            if (!middle_ftd.interior_textures.empty()) {
                float repeated_height =
                    building_top
                    - (has_socle * socle_height)
                    - 2 * middle_ftd.interior_textures.facade_edge_size(1)
                    + middle_ftd.interior_textures.facade_inner_size(1);
                if (repeated_height <= 0) {
                    throw std::runtime_error("Building too small for socle height and facade edge size");
                }
                building_top -= std::fmod(
                    repeated_height,
                    middle_ftd.interior_textures.interior_size(1) +
                    middle_ftd.interior_textures.facade_inner_size(1));
            } else {
                float repeated_height = building_top - (has_socle * socle_height);
                if (repeated_height <= 0) {
                    throw std::runtime_error("Building too small for socle height and uv-scale");
                }
                building_top -= std::fmod(repeated_height, 1.f / uv_scale_facade);
            }
        }
        std::optional<Roof9_2> roof_9_2;
        if (tags.contains("3dr:type", "9.2")) {
            float roof_height = parse_meters(tags, "3dr:height1", NAN);
            float roof_angle = parse_radians(tags, "3dr:alpha", NAN);
            if (std::isnan(roof_height)) {
                throw std::runtime_error("3dr:type=9.2 requires 3dr:height1");
            }
            if (std::isnan(roof_angle)) {
                throw std::runtime_error("3dr:type=9.2 requires 3dr:alpha");
            }
            roof_9_2 = Roof9_2{
                .width = (std::abs(roof_angle - float{M_PI / 2}) < 1e-3)
                    ? 0.f
                    : roof_height / std::tan(roof_angle),
                .height = roof_height};
        }
        if (has_socle)
        {
            if (socle_textures.empty()) {
                throw std::runtime_error("Socle textures empty");
            }
            if (building_top <= socle_height) {
                throw std::runtime_error("Building height too small for socle. ID=" + w.first);
            }
            result.push_back(Building{
                .id = w.first,
                .way = w.second,
                .levels = {
                    BuildingLevel{
                        .top = socle_height,
                        .bottom = building_bottom,
                        .extra_width = 0.f,
                        .type = BuildingLevelType::SOCLE,
                        .facade_texture_descriptor = FacadeTextureDescriptor{
                            .name = socle_textures.at(bid % socle_textures.size())
                        }},
                    BuildingLevel{
                        .top = building_top,
                        .bottom = socle_height,
                        .type = BuildingLevelType::MIDDLE,
                        .facade_texture_descriptor = middle_ftd}
                },
                .roof_9_2 = roof_9_2,
                .style = style});
        } else if ((vs == tags.end()) || (vs->second == "no")) {
            result.push_back(Building{
                .id = w.first,
                .way = w.second,
                .levels = {BuildingLevel{
                    .top = building_top,
                    .bottom = building_bottom,
                    .type = BuildingLevelType::MIDDLE,
                    .facade_texture_descriptor = middle_ftd
                }},
                .roof_9_2 = roof_9_2,
                .style = style});
        } else {
            throw std::runtime_error("Unknown vertical_subdivision: " + vs->second);
        }
        ++bid;
    }
    return result;
}
