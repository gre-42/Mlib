#include "Get_Buildings_Or_Wall_Barriers.hpp"
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Contour_Is_Ok.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Limits.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Socle_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertical_Subdivision.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <ranges>
#include <stdexcept>

using namespace Mlib;

static const float MINIMUM_HEIGHT = 0.1f;

std::list<Building> Mlib::get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<std::string, Relation>& relations,
    double scale,
    double max_length,
    float building_bottom,
    float default_building_top,
    bool default_snap_height,
    float uv_scale_facade,
    float socle_height,
    const std::vector<SocleTexture>& socle_textures,
    float default_roof_9_2_max_building_height,
    const Roof9_2* default_roof_9_2,
    FacadeTextureCycle& entrance_ftc,
    FacadeTextureCycle& middle_ftc,
    VerticalSubdivision default_vertical_subdivision)
{
    std::map<std::string, std::string> child_2_parent;
    std::map<std::string, Building*> id_2_building;
    std::map<Building*, std::string> building_2_id;
    if (building_type == BuildingType::BUILDING) {
        for (const auto& [rid, relation] : relations) {
            if (relation.tags.contains("type", "building")) {
                auto is_outline = [](const Member& e){ return e.role == "outline"; };
                auto outlines =
                    relation.members |
                    std::views::filter(is_outline) |
                    std::ranges::to<std::vector>();
                if (outlines.size() != 1) {
                    throw std::runtime_error(std::format("Could not find unique outline in relation \"{}\"", rid));
                }
                for (const auto& member : relation.members) {
                    if (member.role == "part") {
                        child_2_parent[member.ref] = outlines[0].ref;
                    }
                }
            }
        }
    }
    size_t bid = 0;
    std::list<Building> result;
    for (const auto& [id, w] : ways) {
        if (w.tags.contains("layer") &&
            (safe_stoi(w.tags.at("layer")) != 0)) {
            continue;
        }
        if (w.tags.contains("aeroway", "terminal")) {
            if ((building_type != BuildingType::BUILDING) || excluded_buildings.contains("terminal")) {
                continue;
            }
        } else if (w.tags.find("building") != w.tags.end()) {
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
        std::string middle_style;
        std::string entrance_style;
        switch (building_type) {
            case BuildingType::BUILDING:
                middle_style = w.tags.get("building:material", "");
                entrance_style = w.tags.get("building:entrance_material", "");
                break;
            case BuildingType::WALL_BARRIER:
                middle_style = w.tags.get("style", "");
                break;
            case BuildingType::SPAWN_LINE:
                break;
            default:
                throw std::runtime_error("Unknown building type");
        }
        float building_top = default_building_top;
        building_top = parse_meters(w.tags, "height", building_top);
        building_top = parse_meters(w.tags, "building:height", building_top);

        auto vss = w.tags.try_get("vertical_subdivision");
        auto vertical_subdivision = (vss != nullptr)
            ? vertical_subdivision_from_string(*vss)
            : default_vertical_subdivision;

        FacadeTextureDescriptor entrance_ftd;
        FacadeTextureDescriptor middle_ftd;
        if (building_type == BuildingType::BUILDING) {
            if (any(vertical_subdivision & VerticalSubdivision::ANY_ENTRANCES)) {
                entrance_ftd = entrance_ftc(entrance_style, building_top).descriptor;
            }
            middle_ftd = middle_ftc(middle_style, building_top).descriptor;
        }
        float entrances_height =
            entrance_ftd.interior_textures.interior_size(1) +
            2.f * entrance_ftd.interior_textures.facade_edge_size(1);
        float base_height = building_bottom;
        if (any(vertical_subdivision & VerticalSubdivision::ANY_SOCLE)) {
            base_height += socle_height;
        }
        if (any(vertical_subdivision & VerticalSubdivision::ANY_ENTRANCES)) {
            base_height += entrances_height;
        }
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
            vertical_subdivision = VerticalSubdivision::NONE;
        } else {
            building_top += delta_height;
        }
        std::optional<Roof9_2> roof_9_2;
        if (w.tags.contains("3dr:type", "9.2")) {
            float roof_height = parse_meters(w.tags, "3dr:height1", NAN);
            float roof_angle = parse_radians(w.tags, "3dr:alpha", NAN);
            if (std::isnan(roof_height)) {
                throw std::runtime_error("3dr:type=9.2 requires 3dr:height1");
            }
            if (std::isnan(roof_angle)) {
                throw std::runtime_error("3dr:type=9.2 requires 3dr:alpha");
            }
            roof_9_2 = Roof9_2{
                .width = (std::abs(roof_angle - float(M_PI / 2.)) < float(1e-3))
                    ? 0.f
                    : roof_height / std::tan(roof_angle),
                .height = roof_height};
        } else if (
            (building_top <= default_roof_9_2_max_building_height) &&
            (default_roof_9_2 != nullptr))
        {
            roof_9_2 = *default_roof_9_2;
        }
        std::list<BuildingLevel> levels;
        float previous_height = building_bottom;
        auto add_level = [&](BuildingLevel&& level){
            levels.push_back(std::move(level));
            previous_height = level.top;
        };
        if (any(vertical_subdivision & VerticalSubdivision::ANY_SOCLE)) {
            if (socle_textures.empty()) {
                throw std::runtime_error("Socle textures empty");
            }
            const auto& st = socle_textures.at(bid % socle_textures.size());
            add_level(
                BuildingLevel{
                    .top = previous_height + socle_height,
                    .bottom = previous_height,
                    .extra_width = 0.f,
                    .type = BuildingLevelType::SOCLE,
                    .facade_texture_descriptor = FacadeTextureDescriptor{
                        .names = st.textures,
                        .material = st.material
                    }});
        }
        if (any(vertical_subdivision & VerticalSubdivision::ANY_ENTRANCES)) {
            add_level(
                BuildingLevel{
                    .top = previous_height + entrances_height,
                    .bottom = previous_height,
                    .extra_width = 0.f,
                    .type = BuildingLevelType::ENTRANCES,
                    .facade_texture_descriptor = entrance_ftd});
        }
        if (building_type != BuildingType::SPAWN_LINE) {
            add_level(
                BuildingLevel{
                    .top = building_top,
                    .bottom = previous_height,
                    .type = BuildingLevelType::MIDDLE,
                    .facade_texture_descriptor = middle_ftd});
        }
        {
            float last_height = -INFINITY;
            for (const auto& l : levels) {
                if (l.bottom < last_height) {
                    throw std::runtime_error("Inconsistent building height. ID: \"" + id + '"');
                }
                if (l.top <= l.bottom) {
                    throw std::runtime_error("Building level has no positive height. ID: \"" + id + '"');
                }
                last_height = l.top;
            }
        }
        auto& bu = result.emplace_back(Building{
            .id = id,
            .way = w,
            .levels = std::move(levels),
            .min_level = safe_sto<uint32_t>(w.tags.get("building:min_level", "0")),
            .roof_9_2 = roof_9_2,
            .area = (building_type == BuildingType::BUILDING)
                ? (float)compute_area_clockwise(w.nd, nodes, scale)
                : NAN,
            .style = middle_style,
            .parent = nullptr});
        id_2_building[id] = &bu;
        building_2_id[&bu] = id;
        if (bu.roof_9_2.has_value()) {
            auto sw = smooth_building_level_outline(
                bu,
                nodes,
                scale,
                max_length,
                DrawBuildingPartType::CEILING,
                BuildingDetailType::HIGH);
            std::vector<FixedArray<CompressedScenePos, 2>> outline;
            outline.reserve(sw.outline.size());
            for (const auto& v : sw.outline) {
                outline.emplace_back(v.indented);
            }
            if (!contour_is_ok(outline, MIN_VERTEX_DISTANCE)) {
                lwarn() << "Building \"" << id << "\" has intersecting roof segments";
                bu.roof_9_2.reset();
            }
        }
        ++bid;
    }
    for (auto& c : result) {
        auto cid = building_2_id.find(&c);
        if (cid == building_2_id.end()) {
            continue;
        }
        auto pid = child_2_parent.find(cid->second);
        if (pid == child_2_parent.end()) {
            continue;
        }
        auto pd = id_2_building.find(pid->second);
        if (pd == id_2_building.end()) {
            throw std::runtime_error(std::format("Could not find building for ID \"{}\"", pid->second));
        }
        c.parent = pd->second;
    }
    return result;
}
