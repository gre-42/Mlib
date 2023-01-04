#include "Load_Osm_Resource.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/Trim.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>

static uint32_t CACHE_FILE_VERSION = 6;

namespace fs = std::filesystem;

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ARGUMENTS);

LoadSceneUserFunction LoadOsmResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*osm_resource\\s+([\\s\\S]+)");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void LoadOsmResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    static const DECLARE_REGEX(wayside_resource_names_reg,
        "^\\s*min_dist:([\\w+-.]+)"
        "\\s+max_dist:([\\w+-.]+)"
        "([\\s:\\w+-. \\(\\)/]*)$");
    auto fpathp = [&](const std::string& v){return args.fpath(v).path;};

    OsmResourceConfig config;
    std::string cache_filename;
    static const DECLARE_REGEX(key_value_reg, "(?:\\s*([^=]+)=\\s*([^,]*),?|([\\s\\S]+))");
    std::string resource_name;
    std::vector<float> layer_heights_layer;
    std::vector<float> layer_heights_height;
    find_all(match[1].str(), key_value_reg, [&, &scene_node_resources=args.scene_node_resources](const Mlib::re::smatch& match2) {
        if (match2[3].matched) {
            THROW_OR_ABORT("Unknown element: \"" + match2[3].str() + '"');
        }
        std::string key = rtrim_copy(match2[1].str());
        std::string value = rtrim_copy(match2[2].str());
        auto add_street_textures = [&value, &fpathp, &config](RoadType road_type){
            static const DECLARE_REGEX(
                street_texture_reg,
                "(?:\\s*lanes:(\\d+) "
                "texture0:(#?[\\w+-.\\(\\)/]+)"
                "(?: texture1:(#?[\\w+-.\\(\\)/]+))?"
                "(?: uvx:([\\w+-.]+))?|"
                "([\\s\\S]+))");
            find_all(value, street_texture_reg, [&](const Mlib::re::smatch& match3) {
                if (match3[5].matched) {
                    THROW_OR_ABORT("Unknown element: \"" + match3[5].str() + '"');
                }
                RoadProperties rp{.type=road_type, .nlanes = safe_stoz(match3[1].str())};
                std::vector<std::string> textures =
                    match3[3].matched
                        ? std::vector<std::string>{ fpathp(match3[2].str()), fpathp(match3[3].str()) }
                        : std::vector<std::string>{ fpathp(match3[2].str()) };
                RoadStyle rs{
                    .textures = textures,
                    .uvx = match3[4].matched
                        ? safe_stof(match3[4].str())
                        : 1.f};
                config.street_texture[rp] = rs;
            });
        };
        auto add_styles = [&value, &fpathp, &config](std::map<std::string, BarrierStyle>& styles){
            static const DECLARE_REGEX(
                barrier_texture_reg,
                "(?:\\s*name:(\\w+) "
                "texture:(#?[\\w+-.\\(\\)/]+) "
                "uv:([\\w+-.]+)\\s+([\\w+-.]+) "
                "blend_mode:(\\w+) "
                "wrap_mode_t:(repeat|clamp_to_edge|clamp_to_border) "
                "reorient_uv0:(0|1) "
                "ambience:([\\w+-.]+) "
                "diffusivity:([\\w+-.]+) "
                "specularity:([\\w+-.]+)|([\\s\\S]+))");
            find_all(value, barrier_texture_reg, [&](const Mlib::re::smatch& match3) {
                if (match3[11].matched) {
                    THROW_OR_ABORT("Unknown element: \"" + match3[11].str() + '"');
                }
                BarrierStyle as{
                    .texture = fpathp(match3[2].str()),
                    .uv = FixedArray<float, 2>{
                        safe_stof(match3[3].str()),
                        safe_stof(match3[4].str())},
                    .blend_mode = blend_mode_from_string(match3[5].str()),
                    .wrap_mode_t = wrap_mode_from_string(match3[6].str()),
                    .reorient_uv0 = safe_stob(match3[7].str()),
                    .ambience = safe_stof(match3[8].str()),
                    .diffusivity = safe_stof(match3[9].str()),
                    .specularity = safe_stof(match3[10].str())};
                if (!styles.insert({match3[1].str(), as}).second) {
                    THROW_OR_ABORT("Duplicate barrier style");
                }
            });
        };
        if (key.starts_with("#")) {
            // Do nothing
            return;
        }
        if (key == "name") {
            resource_name = value;
            return;
        }
        if (key == "filename") {
            config.filename = fpathp(value);
            return;
        }
        if (key == "cache_filename") {
            cache_filename = fpathp(value);
            return;
        }
        if (key == "heightmap") {
            config.heightmap = fpathp(value);
            return;
        }
        if (key == "heightmap_mask") {
            config.heightmap_mask = fpathp(value);
            return;
        }
        if (key == "heightmap_extension") {
            config.heightmap_extension = safe_stoz(value);
            return;
        }
        if (key == "terrain_materials") {
            config.terrain_material = physics_material_from_string(value);
            return;
        }
        if (key == "street_materials") {
            config.street_material = physics_material_from_string(value);
            return;
        }
        if (key == "terrain_undefined_textures") {
            config.terrain_textures[TerrainType::UNDEFINED] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_grass_textures") {
            config.terrain_textures[TerrainType::GRASS] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_flowers_textures") {
            config.terrain_textures[TerrainType::FLOWERS] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_trees_textures") {
            config.terrain_textures[TerrainType::TREES] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_elevated_grass_textures") {
            config.terrain_textures[TerrainType::ELEVATED_GRASS] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_elevated_grass_base_textures") {
            config.terrain_textures[TerrainType::ELEVATED_GRASS_BASE] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_stone_textures") {
            config.terrain_textures[TerrainType::STONE] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_asphalt_textures") {
            config.terrain_textures[TerrainType::ASPHALT] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_water_floor_textures") {
            config.terrain_textures[TerrainType::WATER_FLOOR] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "terrain_water_floor_base_textures") {
            config.terrain_textures[TerrainType::WATER_FLOOR_BASE] = string_to_vector(value, fpathp);
            return;
        }
        if (key == "stone_dirt_texture") {
            config.terrain_dirt_textures[TerrainType::STONE] = fpathp(value);
            return;
        }
        if (key == "grass_dirt_texture") {
            config.terrain_dirt_textures[TerrainType::GRASS] = fpathp(value);
            return;
        }
        if (key == "flowers_dirt_texture") {
            config.terrain_dirt_textures[TerrainType::FLOWERS] = fpathp(value);
            return;
        }
        if (key == "trees_dirt_texture") {
            config.terrain_dirt_textures[TerrainType::TREES] = fpathp(value);
            return;
        }
        if (key == "street_dirt_texture") {
            config.street_dirt_texture = fpathp(value);
            return;
        }
        if (key == "street_reflection_map") {
            config.street_reflection_map = value;
            return;
        }
        if (key == "street_crossing_texture") {
            config.street_crossing_texture[RoadType::STREET] = fpathp(value);
            return;
        }
        if (key == "street_texture") {
            RoadProperties rp{.type=RoadType::STREET, .nlanes = 1};
            RoadStyle rs{.textures = { fpathp(value) }, .uvx = 1.f};
            config.street_texture[rp] = rs;
            return;
        }
        if (key == "street_textures") {
            add_street_textures(RoadType::STREET);
            return;
        }
        if (key == "path_crossing_texture") {
            config.street_crossing_texture[RoadType::PATH] = fpathp(value);
            return;
        }
        if (key == "path_texture") {
            RoadProperties rp{.type=RoadType::PATH, .nlanes = 1};
            RoadStyle rs{.textures = { fpathp(value) }, .uvx = 1.f};
            config.street_texture[rp] = rs;
            return;
        }
        if (key == "path_textures") {
            add_street_textures(RoadType::PATH);
            return;
        }
        if (key == "wall_textures") {
            add_street_textures(RoadType::WALL);
            return;
        }
        if (key == "curb_street_texture") {
            config.curb_street_texture[RoadType::STREET] = fpathp(value);
            return;
        }
        if (key == "curb_path_texture") {
            config.curb_street_texture[RoadType::PATH] = fpathp(value);
            return;
        }
        if (key == "curb_wall_texture") {
            config.curb_street_texture[RoadType::WALL] = fpathp(value);
            return;
        }
        if (key == "curb2_street_texture") {
            config.curb2_street_texture[RoadType::STREET] = fpathp(value);
            return;
        }
        if (key == "curb2_path_texture") {
            config.curb2_street_texture[RoadType::PATH] = fpathp(value);
            return;
        }
        if (key == "curb2_wall_texture") {
            config.curb2_street_texture[RoadType::WALL] = fpathp(value);
            return;
        }
        if (key == "air_curb_street_texture") {
            config.air_curb_street_texture[RoadType::STREET] = fpathp(value);
            return;
        }
        if (key == "air_curb_path_texture") {
            config.air_curb_street_texture[RoadType::PATH] = fpathp(value);
            return;
        }
        if (key == "air_support_texture") {
            config.air_support_texture = fpathp(value);
            return;
        }
        if (key == "racing_line_texture") {
            config.racing_line_texture = fpathp(value);
            return;
        }
        if (key == "racing_line_track") {
            config.racing_line_track = fpathp(value);
            return;
        }
        if (key == "racing_line_playback") {
            config.racing_line_playback = fpathp(value);
            return;
        }
        if (key == "socle_textures") {
            config.socle_textures = string_to_vector(value, fpathp);
            return;
        }
        if (key == "facade_textures") {
            config.facade_textures.push_back(parse_facade_texture(value));
            return;
        }
        if (key == "ceiling_texture") {
            config.ceiling_texture = fpathp(value);
            return;
        }
        if (key == "barrier_styles") {
            add_styles(config.barrier_styles);
            return;
        }
        if (key == "roof_texture") {
            config.roof_texture = fpathp(value);
            return;
        }
        if (key == "tunnel_pipe_texture") {
            config.tunnel_pipe_texture = fpathp(value);
            return;
        }
        if (key == "tunnel_pipe_resource_name") {
            config.tunnel_pipe_resource_name = value;
            return;
        }
        if (key == "tunnel_bdry_resource_name") {
            config.tunnel_bdry_resource_name = value;
            return;
        }
        if (key == "street_surface_central_resource_name") {
            config.street_surface_central_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "street_surface_endpoint0_resource_name") {
            config.street_surface_endpoint0_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "street_surface_endpoint1_resource_name") {
            config.street_surface_endpoint1_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "path_surface_central_resource_name") {
            config.street_surface_central_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "path_surface_endpoint0_resource_name") {
            config.street_surface_endpoint0_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "path_surface_endpoint1_resource_name") {
            config.street_surface_endpoint1_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "street_bumps_central_resource_name") {
            config.street_bumps_central_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "street_bumps_endpoint0_resource_name") {
            config.street_bumps_endpoint0_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "street_bumps_endpoint1_resource_name") {
            config.street_bumps_endpoint1_resource_names[RoadType::STREET] = value;
            return;
        }
        if (key == "path_bumps_central_resource_name") {
            config.street_bumps_central_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "path_bumps_endpoint0_resource_name") {
            config.street_bumps_endpoint0_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "path_bumps_endpoint1_resource_name") {
            config.street_bumps_endpoint1_resource_names[RoadType::PATH] = value;
            return;
        }
        if (key == "water_texture") {
            config.water_texture = fpathp(value);
            return;
        }
        if (key == "water_height") {
            config.water_height = safe_stof(value);
            return;
        }
        if (key == "tree_resource_names") {
            config.tree_resource_names.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "grass_resource_names") {
            config.grass_resource_names.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_grass_resource_names") {
            config.near_grass_terrain_style_config.near_resource_names_valley.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_wayside1_grass_resource_names") {
            config.near_wayside1_grass_terrain_style_config.near_resource_names_valley.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_wayside2_grass_resource_names") {
            config.near_wayside2_grass_terrain_style_config.near_resource_names_valley.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_rocks_resource_names") {
            config.near_grass_terrain_style_config.near_resource_names_mountain.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_wayside1_rocks_resource_names") {
            config.near_wayside1_grass_terrain_style_config.near_resource_names_mountain.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_wayside2_rocks_resource_names") {
            config.near_wayside2_grass_terrain_style_config.near_resource_names_mountain.push_back(parse_resource_name(scene_node_resources, value));
            return;
        }
        if (key == "near_flowers_resource_names") {
            config.near_flowers_terrain_style_config.near_resource_names_valley = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
            return;
        }
        if (key == "near_trees_resource_names") {
            config.near_trees_terrain_style_config.near_resource_names_valley = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
            return;
        }
        if (key == "dirt_decals_resource_names") {
            config.no_grass_decals_terrain_style_config.near_resource_names_valley = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
            return;
        }
        if (key == "wayside_resource_names") {
            Mlib::re::smatch match3;
            if (!Mlib::re::regex_match(value, match3, wayside_resource_names_reg)) {
                THROW_OR_ABORT("Could not parse \"" + value + '"');
            }
            config.waysides.push_back(WaysideResourceNames{
                .min_dist = safe_stof(match3[1].str()),
                .max_dist = safe_stof(match3[2].str()),
                .resource_names = string_to_vector(match3[3].str(), [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);}) });
            return;
        }
        if (key == "extrusion_ambient_occlusion") {
            config.extrusion_ambient_occlusion = safe_stof(value);
            return;
        }
        if (key == "laplace_ambient_occlusion") {
            config.laplace_ambient_occlusion = safe_stof(value);
            return;
        }
        if (key == "bounding_terrain_type") {
            config.bounding_terrain_type = terrain_type_from_string(value);
            return;
        }
        if (key == "default_terrain_type") {
            config.default_terrain_type = terrain_type_from_string(value);
            return;
        }
        if (key == "default_street_width") {
            config.default_street_width = safe_stof(value);
            return;
        }
        if (key == "default_lane_width") {
            config.default_lane_width = safe_stof(value);
            return;
        }
        if (key == "default_tunnel_pipe_width") {
            config.default_tunnel_pipe_width = safe_stof(value);
            return;
        }
        if (key == "default_tunnel_pipe_height") {
            config.default_tunnel_pipe_height = safe_stof(value);
            return;
        }
        if (key == "scale") {
            config.scale = safe_stof(value);
            return;
        }
        if (key == "height_scale") {
            config.height_scale = safe_stof(value);
            return;
        }
        if (key == "uv_scale_terrain") {
            config.uv_scale_terrain = safe_stof(value);
            return;
        }
        if (key == "uv_period_terrain") {
            config.uv_period_terrain = safe_stof(value);
            return;
        }
        if (key == "uv_scale_street") {
            config.uv_scales_street[RoadType::STREET] = safe_stof(value);
            return;
        }
        if (key == "uv_scale_path") {
            config.uv_scales_street[RoadType::PATH] = safe_stof(value);
            return;
        }
        if (key == "uv_scale_wall") {
            config.uv_scales_street[RoadType::WALL] = safe_stof(value);
            return;
        }
        if (key == "uv_scale_facade") {
            config.uv_scale_facade = safe_stof(value);
            return;
        }
        if (key == "uv_scale_ceiling") {
            config.uv_scale_ceiling = safe_stof(value);
            return;
        }
        if (key == "uv_scale_barrier_wall") {
            config.uv_scale_barrier_wall = safe_stof(value);
            return;
        }
        if (key == "uv_scale_highway_wall") {
            config.uv_scale_highway_wall = safe_stof(value);
            return;
        }
        if (key == "uv_scale_crossings") {
            config.uv_scale_crossings = safe_stof(value);
            return;
        }
        if (key == "with_roofs") {
            config.with_roofs = safe_stob(value);
            return;
        }
        if (key == "with_ceilings") {
            config.with_ceilings = safe_stob(value);
            return;
        }
        if (key == "building_bottom") {
            config.building_bottom = safe_stof(value);
            return;
        }
        if (key == "default_building_top") {
            config.default_building_top = safe_stof(value);
            return;
        }
        if (key == "default_barrier_top") {
            config.default_barrier_top = safe_stof(value);
            return;
        }
        if (key == "remove_backfacing_triangles") {
            config.remove_backfacing_triangles = safe_stob(value);
            return;
        }
        if (key == "with_tree_nodes") {
            config.with_tree_nodes = safe_stob(value);
            return;
        }
        if (key == "forest_outline_tree_distance") {
            config.forest_outline_tree_distance = safe_stof(value);
            return;
        }
        if (key == "forest_outline_tree_inwards_distance") {
            config.forest_outline_tree_inwards_distance = safe_stof(value);
            return;
        }
        if (key == "much_grass_distance") {
            config.much_grass_distance = safe_stof(value);
            return;
        }
        if (key == "much_near_grass_distance") {
            config.near_grass_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "much_near_wayside1_grass_distance") {
            config.near_wayside1_grass_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "much_near_wayside2_grass_distance") {
            config.near_wayside2_grass_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "much_near_flowers_distance") {
            config.near_flowers_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "much_near_trees_distance") {
            config.near_trees_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "dirt_decals_distance") {
            config.no_grass_decals_terrain_style_config.much_near_distance = safe_stof(value);
            return;
        }
        if (key == "raceway_beacon_distance") {
            config.raceway_beacon_distance = safe_stof(value);
            return;
        }
        if (key == "with_terrain") {
            config.with_terrain = safe_stob(value);
            return;
        }
        if (key == "with_buildings") {
            config.with_buildings = safe_stob(value);
            return;
        }
        if (key == "only_raceways_and_walls") {
            config.only_raceways_and_walls = safe_stob(value);
            return;
        }
        if (key == "with_street_way_points") {
            config.with_street_way_points = safe_stob(value);
            return;
        }
        if (key == "with_sidewalk_way_points") {
            config.with_sidewalk_way_points = safe_stob(value);
            return;
        }
        if (key == "highway_name_pattern") {
            config.highway_name_pattern = value;
            return;
        }
        if (key == "excluded_highways") {
            config.excluded_highways = string_to_set(value);
            return;
        }
        if (key == "path_tags") {
            config.path_tags = string_to_set(value);
            return;
        }
        if (key == "smoothed_highways") {
            config.smoothed_highways = string_to_set(value);
            return;
        }
        if (key == "max_smooth_highway_length") {
            config.max_smooth_highway_length = safe_stof(value);
            return;
        }
        if (key == "steiner_point_distances_road") {
            config.steiner_point_distances_road = string_to_vector(value, safe_stod);
            return;
        }
        if (key == "steiner_point_distances_steiner") {
            config.steiner_point_distances_steiner = string_to_vector(value, safe_stod);
            return;
        }
        if (key == "curb_alpha") {
            config.curb_alpha = safe_stof(value);
            return;
        }
        if (key == "curb2_alpha") {
            config.curb2_alpha = safe_stof(value);
            return;
        }
        if (key == "curb_uv_x") {
            config.curb_uv(0) = safe_stof(value);
            return;
        }
        if (key == "curb_uv_y") {
            config.curb_uv(1) = safe_stof(value);
            return;
        }
        if (key == "curb2_uv_x") {
            config.curb2_uv(0) = safe_stof(value);
            return;
        }
        if (key == "curb2_uv_y") {
            config.curb2_uv(1) = safe_stof(value);
            return;
        }
        if (key == "curb_color") {
            auto curb_color = string_to_vector(value, safe_stof, 3);
            config.curb_color = FixedArray<float, 3>{ curb_color[0], curb_color[1], curb_color[2] };
            return;
        }
        if (key == "racing_line_width_x") {
            config.racing_line_width_x = safe_stof(value);
            return;
        }
        if (key == "racing_line_scale_y") {
            config.racing_line_scale_y = safe_stof(value);
            return;
        }
        if (key == "raise_streets_amount") {
            config.raise_streets_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_curb_amount") {
            config.extrude_curb_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_street_amount") {
            config.extrude_street_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_air_curb_amount") {
            config.extrude_air_curb_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_air_support_amount") {
            config.extrude_air_support_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_wall_amount") {
            config.extrude_wall_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_grass_amount") {
            config.extrude_grass_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_elevated_grass_amount") {
            config.extrude_elevated_grass_amount = safe_stof(value);
            return;
        }
        if (key == "extrude_water_floor_amout") {
            config.extrude_water_floor_amout = safe_stof(value);
            return;
        }
        if (key == "street_light_resource_names") {
            config.street_light_resource_names = string_to_vector(value, [&scene_node_resources](const std::string& name){return parse_resource_name(scene_node_resources, name);});
            return;
        }
        if (key == "max_wall_width") {
            config.max_wall_width = safe_stof(value);
            return;
        }
        if (key == "with_height_bindings") {
            config.with_height_bindings = safe_stob(value);
            return;
        }
        if (key == "street_node_smoothness") {
            config.street_node_smoothness = 1.f - (float)std::pow(10, -safe_stof(value));
            return;
        }
        if (key == "street_node_smoothing_iterations") {
            config.street_node_smoothing_iterations = safe_stoz(value);
            return;
        }
        if (key == "street_edge_smoothness") {
            config.street_edge_smoothness = safe_stof(value);
            return;
        }
        if (key == "terrain_edge_smoothness") {
            config.terrain_edge_smoothness = safe_stof(value);
            return;
        }
        if (key == "bump_height") {
            config.bump_height = safe_stof(value);
            return;
        }
        if (key == "driving_direction") {
            config.driving_direction = driving_direction_from_string(value);
            return;
        }
        if (key == "blend_street") {
            config.blend_street = safe_stob(value);
            return;
        }
        if (key == "layer_heights_layer") {
            layer_heights_layer = string_to_vector(value, safe_stof);
            return;
        }
        if (key == "layer_heights_height") {
            layer_heights_height = string_to_vector(value, safe_stof);
            return;
        }
        if (key == "game_level") {
            config.game_level = value;
            return;
        }
        if (key == "base_osm_map_resource") {
            config.base_osm_map_resource = value;
            return;
        }
        if (key == "navmesh_resource") {
            config.navmesh_resource = value;
            return;
        }
        if (key == "agent_radius") {
            config.agent_radius = safe_stof(value);
            return;
        }
        if (key == "refine_explicit_waypoints") {
            config.refine_explicit_waypoints = safe_stob(value);
            return;
        }
        THROW_OR_ABORT("Unknown osm key: \"" + key + '"');
    });
    if (resource_name.empty()) {
        THROW_OR_ABORT("Osm resource name not set");
    }
    config.layer_heights = Interp<float>(
        layer_heights_layer,
        layer_heights_height);
    if (cache_filename.empty()) {
        THROW_OR_ABORT("Cache filename not set");
    }
    if (!cache_filename.ends_with(".cereal.binary")) {
        THROW_OR_ABORT("Cache filename does not end with .cereal.binary");
    }
    std::shared_ptr<OsmMapResource> osm_map_resource;
    bool enable_cache = getenv_default_bool("ENABLE_OSM_MAP_CACHE", true);
    std::string cache_version_filename = cache_filename + ".version";
    uint32_t old_cache_file_version = 0;
    static std::mutex cache_file_mutex;
    {
        std::lock_guard lock{cache_file_mutex};
        if (path_exists(cache_version_filename)) {
            auto ifstr = create_ifstream(cache_version_filename);
            if (ifstr->fail()) {
                THROW_OR_ABORT("Could not open cache version file \"" + cache_version_filename + "\" for reading");
            }
            *ifstr >> old_cache_file_version;
            if (ifstr->fail() && !ifstr->eof()) {
                THROW_OR_ABORT("Could not read from cache version file \"" + cache_version_filename + '"');
            }
        }
    }
    if (enable_cache && (old_cache_file_version == CACHE_FILE_VERSION) && path_exists(cache_filename)) {
        osm_map_resource = std::make_shared<OsmMapResource>(
            args.scene_node_resources,
            cache_filename,
            resource_name);
    } else {
        osm_map_resource = std::make_shared<OsmMapResource>(
            args.scene_node_resources,
            config,
            resource_name);
        if (enable_cache) {
            osm_map_resource->save_to_file(cache_filename);
            if (old_cache_file_version != CACHE_FILE_VERSION) {
                std::lock_guard lock{cache_file_mutex};
                auto ofstr = create_ofstream(cache_version_filename);
                if (ofstr->fail()) {
                    THROW_OR_ABORT("Could not open cache version file \"" + cache_version_filename + "\" for writing");
                }
                *ofstr << CACHE_FILE_VERSION;
                if (ofstr->fail()) {
                    THROW_OR_ABORT("Could not write to cache version file \"" + cache_version_filename + '"');
                }
            }
        }
    }
    args.scene_node_resources.add_resource(resource_name, osm_map_resource);
}
