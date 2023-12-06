#include "Get_Buildings_Or_Wall_Barriers.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrances_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Socle_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertical_Subdivision.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const float SOCLE_HEIGHT = 1.2f;
static const float MINIMUM_HEIGHT = 0.1f;

std::list<Building> Mlib::get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top,
    bool default_snap_height,
    float uv_scale_facade,
    const std::vector<SocleTexture>& socle_textures,
    const std::vector<EntrancesTexture>& entrances_textures,
    FacadeTextureCycle& ftc,
    VerticalSubdivision default_vertical_subdivision)
{
    size_t bid = 0;
    std::list<Building> result;
    for (const auto& [id, w] : ways) {
        if (w.tags.contains("layer") &&
            (safe_stoi(w.tags.at("layer")) != 0)) {
            continue;
        }
        if (w.tags.find("building") != w.tags.end()) {
            if ((building_type != BuildingType::BUILDING) || excluded_buildings.contains(w.tags.at("building"))) {
                continue;
            }
        } else if (w.tags.find("barrier") != w.tags.end()) {
            if ((building_type != BuildingType::WALL_BARRIER) || !included_barriers.contains(w.tags.at("barrier"))) {
                continue;
            }
        } else if (w.tags.find("spawn_line") != w.tags.end()) {
            if ((building_type != BuildingType::SPAWN_LINE) || (w.tags.at("spawn_line") != "yes")) {
                continue;
            }
        } else {
            continue;
        }
        std::string style;
        switch (building_type) {
            case BuildingType::BUILDING:
                style = w.tags.contains("building:material") ? w.tags.get("building:material") : "";
                break;
            case BuildingType::WALL_BARRIER:
                style = w.tags.contains("style") ? w.tags.get("style") : "";
                break;
            case BuildingType::SPAWN_LINE:
                break;
            default:
                THROW_OR_ABORT("Unknown building type");
        }
        float building_top = default_building_top;
        building_top = parse_meters(w.tags, "height", building_top);
        building_top = parse_meters(w.tags, "building:height", building_top);

        FacadeTextureDescriptor middle_ftd;
        if (building_type == BuildingType::BUILDING) {
            if (!style.empty()) {
                auto ft = ftc(style);
                if (ft == nullptr) {
                    // THROW_OR_ABORT("Unknown building material: \"" + bu.style + '"');
                    std::cerr << "Unknown building material: \"" + style + '"' << std::endl;
                    middle_ftd = ftc(building_top).descriptor;
                } else {
                    middle_ftd = ft->descriptor;
                }
            } else {
                if (ftc.empty()) {
                    THROW_OR_ABORT("Facade textures empty");
                }
                middle_ftd = ftc(building_top).descriptor;
            }
        }
        auto vss = w.tags.try_get("vertical_subdivision");
        auto vertical_subdivision = vss.has_value()
            ? vertical_subdivision_from_string(vss.value())
            : default_vertical_subdivision;
        float socle_height = SOCLE_HEIGHT * any(vertical_subdivision & VerticalSubdivision::ANY_SOCLE);
        float entrances_height = 1.f / uv_scale_facade * any(vertical_subdivision & VerticalSubdivision::ANY_ENTRANCES);
        float base_height = socle_height + entrances_height;
        float delta_height = 0.f;
        if (w.tags.contains("snap_height", "yes", default_snap_height)) {
            if (!middle_ftd.interior_textures.empty()) {
                float repeated_height =
                    building_top
                    - base_height
                    - 2 * middle_ftd.interior_textures.facade_edge_size(1)
                    + middle_ftd.interior_textures.facade_inner_size(1);
                if (repeated_height <= 0) {
                    lwarn() << "Building too small for socle height and facade edge size. ID=" << id;
                    delta_height = -INFINITY;
                } else {
                    delta_height = -std::fmod(
                        repeated_height,
                        middle_ftd.interior_textures.interior_size(1) +
                        middle_ftd.interior_textures.facade_inner_size(1));
                }
            } else {
                float repeated_height = building_top - base_height;
                if (repeated_height <= 0) {
                    lwarn() << "Building too small for socle height and uv-scale. ID=" << id;
                    delta_height = -INFINITY;
                } else {
                    delta_height = -std::fmod(repeated_height, 1.f / uv_scale_facade);
                }
            }
        }
        if (building_top + delta_height - base_height <= MINIMUM_HEIGHT) {
            lwarn() <<
                "Building height (" << building_top <<
                ") too small for delta height (" << delta_height <<
                ") and base height (" << base_height <<
                "). ID=" << id;
            vertical_subdivision = VerticalSubdivision::NO;
        } else {
            building_top += delta_height;
        }
        std::optional<Roof9_2> roof_9_2;
        if (w.tags.contains("3dr:type", "9.2")) {
            float roof_height = parse_meters(w.tags, "3dr:height1", NAN);
            float roof_angle = parse_radians(w.tags, "3dr:alpha", NAN);
            if (std::isnan(roof_height)) {
                THROW_OR_ABORT("3dr:type=9.2 requires 3dr:height1");
            }
            if (std::isnan(roof_angle)) {
                THROW_OR_ABORT("3dr:type=9.2 requires 3dr:alpha");
            }
            roof_9_2 = Roof9_2{
                .width = (std::abs(roof_angle - float(M_PI / 2.)) < float(1e-3))
                    ? 0.f
                    : roof_height / std::tan(roof_angle),
                .height = roof_height};
        }
        if (vertical_subdivision == VerticalSubdivision::SOCLE) {
            if (socle_textures.empty()) {
                THROW_OR_ABORT("Socle textures empty");
            }
            result.push_back(Building{
                .id = id,
                .way = w,
                .levels = {
                    BuildingLevel{
                        .top = socle_height,
                        .bottom = building_bottom,
                        .extra_width = 0.f,
                        .type = BuildingLevelType::SOCLE,
                        .facade_texture_descriptor = FacadeTextureDescriptor{
                            .names = socle_textures.at(bid % socle_textures.size()).textures
                        }},
                    BuildingLevel{
                        .top = building_top,
                        .bottom = socle_height,
                        .type = BuildingLevelType::MIDDLE,
                        .facade_texture_descriptor = middle_ftd}
                },
                .roof_9_2 = roof_9_2,
                .style = style });
        } else if (vertical_subdivision == VerticalSubdivision::ENTRANCES) {
            if (entrances_textures.empty()) {
                THROW_OR_ABORT("Entrances textures empty");
            }
            const auto& et = entrances_textures.at(bid % entrances_textures.size());
            result.push_back(Building{
                .id = id,
                .way = w,
                .levels = {
                    BuildingLevel{
                        .top = entrances_height,
                        .bottom = building_bottom,
                        .extra_width = 0.f,
                        .type = BuildingLevelType::ENTRANCES,
                        .facade_texture_descriptor = FacadeTextureDescriptor{
                            .names = et.textures,
                            .uv_scale_x = et.uv_scale_x
                        }},
                    BuildingLevel{
                        .top = building_top,
                        .bottom = entrances_height,
                        .type = BuildingLevelType::MIDDLE,
                        .facade_texture_descriptor = middle_ftd}
                    },
                    .roof_9_2 = roof_9_2,
                    .style = style});
        } else if (vertical_subdivision == VerticalSubdivision::NO) {
            result.push_back(Building{
                .id = id,
                .way = w,
                .levels = {BuildingLevel{
                    .top = building_top,
                    .bottom = building_bottom,
                    .type = BuildingLevelType::MIDDLE,
                    .facade_texture_descriptor = middle_ftd
                }},
                .roof_9_2 = roof_9_2,
                .style = style});
        } else {
            verbose_abort("Internal error: Unknown vertical_subdivision: " + (int)vertical_subdivision);
        }
        ++bid;
    }
    return result;
}
