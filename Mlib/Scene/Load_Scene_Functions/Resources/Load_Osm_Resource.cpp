#include "Load_Osm_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/Trim.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>

static uint32_t CACHE_FILE_VERSION = 19;

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(cache_filename);
DECLARE_ARGUMENT(heightmap);
DECLARE_ARGUMENT(heightmap_mask);
DECLARE_ARGUMENT(heightmap_extension);
DECLARE_ARGUMENT(grass_foliagemap);
DECLARE_ARGUMENT(grass_foliagemap_period);
DECLARE_ARGUMENT(terrain_undefined_materials);
DECLARE_ARGUMENT(street_materials);
DECLARE_ARGUMENT(path_materials);
DECLARE_ARGUMENT(wall_materials);
DECLARE_ARGUMENT(terrain_undefined_textures);
DECLARE_ARGUMENT(terrain_grass_textures);
DECLARE_ARGUMENT(terrain_flowers_textures);
DECLARE_ARGUMENT(terrain_trees_textures);
DECLARE_ARGUMENT(terrain_elevated_grass_textures);
DECLARE_ARGUMENT(terrain_elevated_grass_base_textures);
DECLARE_ARGUMENT(terrain_stone_textures);
DECLARE_ARGUMENT(terrain_asphalt_textures);
DECLARE_ARGUMENT(terrain_water_floor_textures);
DECLARE_ARGUMENT(terrain_water_floor_base_textures);
DECLARE_ARGUMENT(terrain_stone_reflection_map);
DECLARE_ARGUMENT(terrain_asphalt_reflection_map);
DECLARE_ARGUMENT(stone_dirt_texture);
DECLARE_ARGUMENT(grass_dirt_texture);
DECLARE_ARGUMENT(flowers_dirt_texture);
DECLARE_ARGUMENT(trees_dirt_texture);
DECLARE_ARGUMENT(street_dirt_texture);
DECLARE_ARGUMENT(street_reflection_map);
DECLARE_ARGUMENT(path_reflection_map);
DECLARE_ARGUMENT(street_crossing_texture);
DECLARE_ARGUMENT(street_texture);
DECLARE_ARGUMENT(street_textures);
DECLARE_ARGUMENT(path_crossing_texture);
DECLARE_ARGUMENT(path_texture);
DECLARE_ARGUMENT(path_textures);
DECLARE_ARGUMENT(wall_textures);
DECLARE_ARGUMENT(curb_street_texture);
DECLARE_ARGUMENT(curb_path_texture);
DECLARE_ARGUMENT(curb_wall_texture);
DECLARE_ARGUMENT(curb2_street_texture);
DECLARE_ARGUMENT(curb2_path_texture);
DECLARE_ARGUMENT(curb2_wall_texture);
DECLARE_ARGUMENT(air_curb_street_texture);
DECLARE_ARGUMENT(air_curb_path_texture);
DECLARE_ARGUMENT(air_support_texture);
DECLARE_ARGUMENT(racing_line_texture);
DECLARE_ARGUMENT(racing_line_track);
DECLARE_ARGUMENT(racing_line_playback);
DECLARE_ARGUMENT(socle_textures);
DECLARE_ARGUMENT(facade_textures);
DECLARE_ARGUMENT(ceiling_texture);
DECLARE_ARGUMENT(barrier_styles);
DECLARE_ARGUMENT(roof_texture);
DECLARE_ARGUMENT(tunnel_pipe_texture);
DECLARE_ARGUMENT(tunnel_pipe_resource_name);
DECLARE_ARGUMENT(tunnel_bdry_resource_name);
DECLARE_ARGUMENT(street_surface_central_resource_name);
DECLARE_ARGUMENT(street_surface_endpoint0_resource_name);
DECLARE_ARGUMENT(street_surface_endpoint1_resource_name);
DECLARE_ARGUMENT(path_surface_central_resource_name);
DECLARE_ARGUMENT(path_surface_endpoint0_resource_name);
DECLARE_ARGUMENT(path_surface_endpoint1_resource_name);
DECLARE_ARGUMENT(street_bumps_central_resource_name);
DECLARE_ARGUMENT(street_bumps_endpoint0_resource_name);
DECLARE_ARGUMENT(street_bumps_endpoint1_resource_name);
DECLARE_ARGUMENT(path_bumps_central_resource_name);
DECLARE_ARGUMENT(path_bumps_endpoint0_resource_name);
DECLARE_ARGUMENT(path_bumps_endpoint1_resource_name);
DECLARE_ARGUMENT(water_texture);
DECLARE_ARGUMENT(water_height);
DECLARE_ARGUMENT(tree_resource_names);
DECLARE_ARGUMENT(grass_resource_names);
DECLARE_ARGUMENT(near_grass_resource_names);
DECLARE_ARGUMENT(near_dirty_grass_resource_names);
DECLARE_ARGUMENT(far_grass_resource_names);
DECLARE_ARGUMENT(far_dirty_grass_resource_names);
DECLARE_ARGUMENT(near_wayside1_grass_resource_names);
DECLARE_ARGUMENT(near_wayside2_grass_resource_names);
DECLARE_ARGUMENT(near_rocks_resource_names);
DECLARE_ARGUMENT(near_wayside1_rocks_resource_names);
DECLARE_ARGUMENT(near_wayside2_rocks_resource_names);
DECLARE_ARGUMENT(near_flowers_resource_names);
DECLARE_ARGUMENT(far_flowers_resource_names);
DECLARE_ARGUMENT(near_trees_resource_names);
DECLARE_ARGUMENT(far_trees_resource_names);
DECLARE_ARGUMENT(dirt_decals_resource_names);
DECLARE_ARGUMENT(wayside_resource_names);
DECLARE_ARGUMENT(extrusion_ambient_occlusion);
DECLARE_ARGUMENT(laplace_ambient_occlusion);
DECLARE_ARGUMENT(bounding_terrain_type);
DECLARE_ARGUMENT(default_terrain_type);
DECLARE_ARGUMENT(default_street_width);
DECLARE_ARGUMENT(default_lane_width);
DECLARE_ARGUMENT(default_tunnel_pipe_width);
DECLARE_ARGUMENT(default_tunnel_pipe_height);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(height_scale);
DECLARE_ARGUMENT(uv_scale_terrain);
DECLARE_ARGUMENT(uv_period_terrain);
DECLARE_ARGUMENT(uv_scale_street);
DECLARE_ARGUMENT(uv_scale_path);
DECLARE_ARGUMENT(uv_scale_wall);
DECLARE_ARGUMENT(uv_scale_facade);
DECLARE_ARGUMENT(uv_scale_ceiling);
DECLARE_ARGUMENT(uv_scale_barrier_wall);
DECLARE_ARGUMENT(uv_scale_highway_wall);
DECLARE_ARGUMENT(uv_scale_crossings);
DECLARE_ARGUMENT(boundary_barrier_height);
DECLARE_ARGUMENT(boundary_barrier_style);
DECLARE_ARGUMENT(with_roofs);
DECLARE_ARGUMENT(with_ceilings);
DECLARE_ARGUMENT(building_bottom);
DECLARE_ARGUMENT(default_building_top);
DECLARE_ARGUMENT(default_barrier_top);
DECLARE_ARGUMENT(remove_backfacing_triangles);
DECLARE_ARGUMENT(with_tree_nodes);
DECLARE_ARGUMENT(forest_outline_tree_distance);
DECLARE_ARGUMENT(forest_outline_tree_inwards_distance);
DECLARE_ARGUMENT(much_grass_distance);
DECLARE_ARGUMENT(much_near_grass_distance);
DECLARE_ARGUMENT(much_far_grass_distance);
DECLARE_ARGUMENT(much_near_wayside1_grass_distance);
DECLARE_ARGUMENT(much_near_wayside2_grass_distance);
DECLARE_ARGUMENT(much_near_flowers_distance);
DECLARE_ARGUMENT(much_far_flowers_distance);
DECLARE_ARGUMENT(much_near_trees_distance);
DECLARE_ARGUMENT(much_far_trees_distance);
DECLARE_ARGUMENT(dirt_decals_distance);
DECLARE_ARGUMENT(raceway_beacon_distance);
DECLARE_ARGUMENT(with_terrain);
DECLARE_ARGUMENT(with_buildings);
DECLARE_ARGUMENT(only_raceways_and_walls);
DECLARE_ARGUMENT(with_street_way_points);
DECLARE_ARGUMENT(with_sidewalk_way_points);
DECLARE_ARGUMENT(highway_name_pattern);
DECLARE_ARGUMENT(excluded_highways);
DECLARE_ARGUMENT(path_tags);
DECLARE_ARGUMENT(smoothed_highways);
DECLARE_ARGUMENT(max_smooth_highway_length);
DECLARE_ARGUMENT(steiner_point_distances_road);
DECLARE_ARGUMENT(steiner_point_distances_steiner);
DECLARE_ARGUMENT(min_dist_to_terrain_region);
DECLARE_ARGUMENT(curb_alpha);
DECLARE_ARGUMENT(curb2_alpha);
DECLARE_ARGUMENT(curb_uv_x);
DECLARE_ARGUMENT(curb_uv_y);
DECLARE_ARGUMENT(curb2_uv_x);
DECLARE_ARGUMENT(curb2_uv_y);
DECLARE_ARGUMENT(curb_color);
DECLARE_ARGUMENT(racing_line_width_x);
DECLARE_ARGUMENT(racing_line_scale_y);
DECLARE_ARGUMENT(raise_streets_amount);
DECLARE_ARGUMENT(extrude_curb_amount);
DECLARE_ARGUMENT(extrude_street_amount);
DECLARE_ARGUMENT(extrude_air_curb_amount);
DECLARE_ARGUMENT(extrude_air_support_amount);
DECLARE_ARGUMENT(extrude_wall_amount);
DECLARE_ARGUMENT(extrude_grass_amount);
DECLARE_ARGUMENT(extrude_elevated_grass_amount);
DECLARE_ARGUMENT(extrude_water_floor_amout);
DECLARE_ARGUMENT(street_light_resource_names);
DECLARE_ARGUMENT(max_wall_width);
DECLARE_ARGUMENT(with_height_bindings);
DECLARE_ARGUMENT(street_node_smoothness);
DECLARE_ARGUMENT(street_node_smoothing_iterations);
DECLARE_ARGUMENT(street_edge_smoothness);
DECLARE_ARGUMENT(terrain_edge_smoothness);
DECLARE_ARGUMENT(bump_height);
DECLARE_ARGUMENT(driving_direction);
DECLARE_ARGUMENT(blend_street);
DECLARE_ARGUMENT(layer_heights_layer);
DECLARE_ARGUMENT(layer_heights_height);
DECLARE_ARGUMENT(game_level);
DECLARE_ARGUMENT(base_osm_map_resource);
DECLARE_ARGUMENT(navmesh_resource);
DECLARE_ARGUMENT(agent_radius);
DECLARE_ARGUMENT(refine_explicit_waypoints);
}

namespace ST {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(lanes);
DECLARE_ARGUMENT(texture0);
DECLARE_ARGUMENT(texture1);
DECLARE_ARGUMENT(uvx);
DECLARE_ARGUMENT(unknown);
}

namespace BS {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(uv);
DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(wrap_mode_t);
DECLARE_ARGUMENT(reorient_uv0);
DECLARE_ARGUMENT(ambience);
DECLARE_ARGUMENT(diffusivity);
DECLARE_ARGUMENT(specularity);
}

namespace WaysideKnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(min_dist);
DECLARE_ARGUMENT(max_dist);
DECLARE_ARGUMENT(resource_names);
}

const std::string LoadOsmResource::key = "osm_resource";

static double from_meters(double v) {
    return v * meters;
}

LoadSceneJsonUserFunction LoadOsmResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();

    auto fpathps = [&args](const std::string& name){
        return args.arguments.pathes_or_variables(name, [](const FPath& v){return v.path;});
    };

    OsmResourceConfig config;
    auto& tconfig = config.triangle_sampler_resource_config;
    std::string cache_filename;
    std::string resource_name;
    std::vector<double> layer_heights_layer;
    std::vector<double> layer_heights_height;
    {
        auto add_street_textures = [&config](RoadType road_type, const std::vector<JsonMacroArguments>& street_textures){
            for (const auto& street_texture : street_textures) {
                street_texture.validate(ST::options, "street texture: ");
                RoadProperties rp{.type=road_type, .nlanes = street_texture.at<size_t>(ST::lanes)};
                std::vector<std::string> textures =
                    street_texture.contains(ST::texture1)
                        ? std::vector<std::string>{ street_texture.path_or_variable(ST::texture0).path, street_texture.path_or_variable(ST::texture1).path }
                        : std::vector<std::string>{ street_texture.path_or_variable(ST::texture0).path };
                RoadStyle rs{
                    .textures = textures,
                    .uvx = street_texture.contains(ST::uvx)
                        ? street_texture.at<float>(ST::uvx)
                        : 1.f};
                config.street_texture[rp] = rs;
            };
        };
        auto add_styles = [](std::map<std::string, BarrierStyle>& styles, const std::vector<JsonMacroArguments>& barrier_styles){
            for (const auto& barrier_style : barrier_styles) {
                barrier_style.validate(BS::options, "barrier style: ");
                BarrierStyle as{
                    .texture = barrier_style.path_or_variable(BS::texture).path,
                    .uv = barrier_style.at(BS::uv).get<FixedArray<float, 2>>(),
                    .blend_mode = blend_mode_from_string(barrier_style.at<std::string>(BS::blend_mode)),
                    .wrap_mode_t = wrap_mode_from_string(barrier_style.at<std::string>(BS::wrap_mode_t)),
                    .reorient_uv0 = barrier_style.at<bool>(BS::reorient_uv0),
                    .ambience = barrier_style.at<float>(BS::ambience),
                    .diffusivity = barrier_style.at<float>(BS::diffusivity),
                    .specularity = barrier_style.at<float>(BS::specularity)};
                if (!styles.insert({barrier_style.at<std::string>(BS::name), as}).second) {
                    THROW_OR_ABORT("Duplicate barrier style");
                }
            };
        };
        auto parse_resource_name_func = [&scene_node_resources](const JsonMacroArguments& jma){
            return parse_resource_name(scene_node_resources, jma.get<std::string>());
        };
        resource_name = args.arguments.at<std::string>(KnownArgs::name);
        config.filename = args.arguments.path(KnownArgs::filename);
        cache_filename = args.arguments.path(KnownArgs::cache_filename);
        if (args.arguments.contains(KnownArgs::heightmap)) {
            config.heightmap = args.arguments.path(KnownArgs::heightmap);
        }
        if (args.arguments.contains(KnownArgs::heightmap_mask)) {
            config.heightmap_mask = args.arguments.path(KnownArgs::heightmap_mask);
        }
        if (args.arguments.contains(KnownArgs::heightmap_extension)) {
            config.heightmap_extension = args.arguments.at<size_t>(KnownArgs::heightmap_extension);
        }
        if (args.arguments.contains(KnownArgs::grass_foliagemap)) {
            tconfig.near_grass_terrain_style_config.foliagemap_filename = args.arguments.path(KnownArgs::grass_foliagemap);
            tconfig.far_grass_terrain_style_config.foliagemap_filename = args.arguments.path(KnownArgs::grass_foliagemap);
        }
        if (args.arguments.contains(KnownArgs::grass_foliagemap_period)) {
            tconfig.near_grass_terrain_style_config.foliagemap_scale = 1.f / args.arguments.at<float>(KnownArgs::grass_foliagemap_period);
            tconfig.far_grass_terrain_style_config.foliagemap_scale = 1.f / args.arguments.at<float>(KnownArgs::grass_foliagemap_period);
        }
        if (args.arguments.contains(KnownArgs::terrain_undefined_materials)) {
            config.terrain_undefined_material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::terrain_undefined_materials));
        }
        if (args.arguments.contains(KnownArgs::street_materials)) {
            config.street_materials[RoadType::STREET] = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::street_materials));
        }
        if (args.arguments.contains(KnownArgs::path_materials)) {
            config.street_materials[RoadType::PATH] = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::path_materials));
        }
        if (args.arguments.contains(KnownArgs::wall_materials)) {
            config.street_materials[RoadType::WALL] = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::wall_materials));
        }
        if (args.arguments.contains(KnownArgs::terrain_undefined_textures)) {
            config.terrain_textures[TerrainType::UNDEFINED] = fpathps(KnownArgs::terrain_undefined_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_grass_textures)) {
            config.terrain_textures[TerrainType::GRASS] = fpathps(KnownArgs::terrain_grass_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_flowers_textures)) {
            config.terrain_textures[TerrainType::FLOWERS] = fpathps(KnownArgs::terrain_flowers_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_trees_textures)) {
            config.terrain_textures[TerrainType::TREES] = fpathps(KnownArgs::terrain_trees_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_elevated_grass_textures)) {
            config.terrain_textures[TerrainType::ELEVATED_GRASS] = fpathps(KnownArgs::terrain_elevated_grass_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_elevated_grass_base_textures)) {
            config.terrain_textures[TerrainType::ELEVATED_GRASS_BASE] = fpathps(KnownArgs::terrain_elevated_grass_base_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_stone_textures)) {
            config.terrain_textures[TerrainType::STONE] = fpathps(KnownArgs::terrain_stone_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_asphalt_textures)) {
            config.terrain_textures[TerrainType::ASPHALT] = fpathps(KnownArgs::terrain_asphalt_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_water_floor_textures)) {
            config.terrain_textures[TerrainType::WATER_FLOOR] = fpathps(KnownArgs::terrain_water_floor_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_water_floor_base_textures)) {
            config.terrain_textures[TerrainType::WATER_FLOOR_BASE] = fpathps(KnownArgs::terrain_water_floor_base_textures);
        }
        if (args.arguments.contains(KnownArgs::terrain_stone_reflection_map)) {
            config.terrain_reflection_map[TerrainType::STONE] = args.arguments.at<std::string>(KnownArgs::terrain_stone_reflection_map);
        }
        if (args.arguments.contains(KnownArgs::terrain_asphalt_reflection_map)) {
            config.terrain_reflection_map[TerrainType::ASPHALT] = args.arguments.at<std::string>(KnownArgs::terrain_asphalt_reflection_map);
        }
        if (args.arguments.contains(KnownArgs::stone_dirt_texture)) {
            config.terrain_dirt_textures[TerrainType::STONE] = args.arguments.path_or_variable(KnownArgs::stone_dirt_texture).path;
        }
        if (args.arguments.contains(KnownArgs::grass_dirt_texture)) {
            config.terrain_dirt_textures[TerrainType::GRASS] = args.arguments.path_or_variable(KnownArgs::grass_dirt_texture).path;
        }
        if (args.arguments.contains(KnownArgs::flowers_dirt_texture)) {
            config.terrain_dirt_textures[TerrainType::FLOWERS] = args.arguments.path_or_variable(KnownArgs::flowers_dirt_texture).path;
        }
        if (args.arguments.contains(KnownArgs::trees_dirt_texture)) {
            config.terrain_dirt_textures[TerrainType::TREES] = args.arguments.path_or_variable(KnownArgs::trees_dirt_texture).path;
        }
        if (args.arguments.contains(KnownArgs::street_dirt_texture)) {
            config.street_dirt_texture = args.arguments.path_or_variable(KnownArgs::street_dirt_texture).path;
        }
        if (args.arguments.contains(KnownArgs::street_reflection_map)) {
            config.street_reflection_map[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_reflection_map);
        }
        if (args.arguments.contains(KnownArgs::path_reflection_map)) {
            config.street_reflection_map[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_reflection_map);
        }
        if (args.arguments.contains(KnownArgs::street_crossing_texture)) {
            config.street_crossing_texture[RoadType::STREET] = args.arguments.path_or_variable(KnownArgs::street_crossing_texture).path;
        }
        if (args.arguments.contains(KnownArgs::street_texture)) {
            RoadProperties rp{.type=RoadType::STREET, .nlanes = 1};
            RoadStyle rs{.textures = { args.arguments.path_or_variable(KnownArgs::street_texture).path }, .uvx = 1.f};
            config.street_texture[rp] = rs;
        }
        if (args.arguments.contains_non_null(KnownArgs::street_textures)) {
            add_street_textures(RoadType::STREET, args.arguments.children(KnownArgs::street_textures));
        }
        if (args.arguments.contains(KnownArgs::path_crossing_texture)) {
            config.street_crossing_texture[RoadType::PATH] = args.arguments.path_or_variable(KnownArgs::path_crossing_texture).path;
        }
        if (args.arguments.contains(KnownArgs::path_texture)) {
            RoadProperties rp{.type=RoadType::PATH, .nlanes = 1};
            RoadStyle rs{.textures = { args.arguments.path_or_variable(KnownArgs::path_texture).path }, .uvx = 1.f};
            config.street_texture[rp] = rs;
        }
        if (args.arguments.contains_non_null(KnownArgs::path_textures)) {
            add_street_textures(RoadType::PATH, args.arguments.children(KnownArgs::path_textures));
        }
        if (args.arguments.contains_non_null(KnownArgs::wall_textures)) {
            add_street_textures(RoadType::WALL, args.arguments.children(KnownArgs::wall_textures));
        }
        if (args.arguments.contains(KnownArgs::curb_street_texture)) {
            config.curb_street_texture[RoadType::STREET] = args.arguments.path_or_variable(KnownArgs::curb_street_texture).path;
        }
        if (args.arguments.contains(KnownArgs::curb_path_texture)) {
            config.curb_street_texture[RoadType::PATH] = args.arguments.path_or_variable(KnownArgs::curb_path_texture).path;
        }
        if (args.arguments.contains(KnownArgs::curb_wall_texture)) {
            config.curb_street_texture[RoadType::WALL] = args.arguments.path_or_variable(KnownArgs::curb_wall_texture).path;
        }
        if (args.arguments.contains(KnownArgs::curb2_street_texture)) {
            config.curb2_street_texture[RoadType::STREET] = args.arguments.path_or_variable(KnownArgs::curb2_street_texture).path;
        }
        if (args.arguments.contains(KnownArgs::curb2_path_texture)) {
            config.curb2_street_texture[RoadType::PATH] = args.arguments.path_or_variable(KnownArgs::curb2_path_texture).path;
        }
        if (args.arguments.contains(KnownArgs::curb2_wall_texture)) {
            config.curb2_street_texture[RoadType::WALL] = args.arguments.path_or_variable(KnownArgs::curb2_wall_texture).path;
        }
        if (args.arguments.contains(KnownArgs::air_curb_street_texture)) {
            config.air_curb_street_texture[RoadType::STREET] = args.arguments.path_or_variable(KnownArgs::air_curb_street_texture).path;
        }
        if (args.arguments.contains(KnownArgs::air_curb_path_texture)) {
            config.air_curb_street_texture[RoadType::PATH] = args.arguments.path_or_variable(KnownArgs::air_curb_path_texture).path;
        }
        if (args.arguments.contains(KnownArgs::air_support_texture)) {
            config.air_support_texture = args.arguments.path_or_variable(KnownArgs::air_support_texture).path;
        }
        if (args.arguments.contains(KnownArgs::racing_line_texture)) {
            config.racing_line_texture = args.arguments.path_or_variable(KnownArgs::racing_line_texture).path;
        }
        if (args.arguments.contains(KnownArgs::racing_line_track)) {
            config.racing_line_track = args.arguments.path_or_variable(KnownArgs::racing_line_track).path;
        }
        if (args.arguments.contains(KnownArgs::racing_line_playback)) {
            config.racing_line_playback = args.arguments.path_or_variable(KnownArgs::racing_line_playback).path;
        }
        if (args.arguments.contains(KnownArgs::socle_textures)) {
            config.socle_textures = fpathps(KnownArgs::socle_textures);
        }
        if (args.arguments.contains_non_null(KnownArgs::facade_textures)) {
            config.facade_textures = args.arguments.children(KnownArgs::facade_textures, parse_facade_texture);
        }
        if (args.arguments.contains(KnownArgs::ceiling_texture)) {
            config.ceiling_texture = args.arguments.path_or_variable(KnownArgs::ceiling_texture).path;
        }
        if (args.arguments.contains_non_null(KnownArgs::barrier_styles)) {
            add_styles(config.barrier_styles, args.arguments.children(KnownArgs::barrier_styles));
        }
        if (args.arguments.contains(KnownArgs::roof_texture)) {
            config.roof_texture = args.arguments.path_or_variable(KnownArgs::roof_texture).path;
        }
        if (args.arguments.contains(KnownArgs::tunnel_pipe_texture)) {
            config.tunnel_pipe_texture = args.arguments.path_or_variable(KnownArgs::tunnel_pipe_texture).path;
        }
        if (args.arguments.contains(KnownArgs::tunnel_pipe_resource_name)) {
            config.tunnel_pipe_resource_name = args.arguments.at<std::string>(KnownArgs::tunnel_pipe_resource_name);
        }
        if (args.arguments.contains(KnownArgs::tunnel_bdry_resource_name)) {
            config.tunnel_bdry_resource_name = args.arguments.at<std::string>(KnownArgs::tunnel_bdry_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_surface_central_resource_name)) {
            config.street_surface_central_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_surface_central_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_surface_endpoint0_resource_name)) {
            config.street_surface_endpoint0_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_surface_endpoint0_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_surface_endpoint1_resource_name)) {
            config.street_surface_endpoint1_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_surface_endpoint1_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_surface_central_resource_name)) {
            config.street_surface_central_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_surface_central_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_surface_endpoint0_resource_name)) {
            config.street_surface_endpoint0_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_surface_endpoint0_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_surface_endpoint1_resource_name)) {
            config.street_surface_endpoint1_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_surface_endpoint1_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_bumps_central_resource_name)) {
            config.street_bumps_central_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_bumps_central_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_bumps_endpoint0_resource_name)) {
            config.street_bumps_endpoint0_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_bumps_endpoint0_resource_name);
        }
        if (args.arguments.contains(KnownArgs::street_bumps_endpoint1_resource_name)) {
            config.street_bumps_endpoint1_resource_names[RoadType::STREET] = args.arguments.at<std::string>(KnownArgs::street_bumps_endpoint1_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_bumps_central_resource_name)) {
            config.street_bumps_central_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_bumps_central_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_bumps_endpoint0_resource_name)) {
            config.street_bumps_endpoint0_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_bumps_endpoint0_resource_name);
        }
        if (args.arguments.contains(KnownArgs::path_bumps_endpoint1_resource_name)) {
            config.street_bumps_endpoint1_resource_names[RoadType::PATH] = args.arguments.at<std::string>(KnownArgs::path_bumps_endpoint1_resource_name);
        }
        if (args.arguments.contains(KnownArgs::water_texture)) {
            config.water_texture = args.arguments.path_or_variable(KnownArgs::water_texture).path;
        }
        if (args.arguments.contains(KnownArgs::water_height)) {
            config.water_height = args.arguments.at<float>(KnownArgs::water_height);
        }
        if (args.arguments.contains_non_null(KnownArgs::tree_resource_names)) {
            config.tree_resource_names = args.arguments.children(KnownArgs::tree_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::grass_resource_names)) {
            config.grass_resource_names = args.arguments.children(KnownArgs::grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_grass_resource_names)) {
            tconfig.near_grass_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::near_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_dirty_grass_resource_names)) {
            tconfig.near_grass_terrain_style_config.near_resource_names_valley_dirt = args.arguments.children(KnownArgs::near_dirty_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::far_grass_resource_names)) {
            tconfig.far_grass_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::far_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::far_dirty_grass_resource_names)) {
            tconfig.far_grass_terrain_style_config.near_resource_names_valley_dirt = args.arguments.children(KnownArgs::far_dirty_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_wayside1_grass_resource_names)) {
            tconfig.near_wayside1_grass_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::near_wayside1_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_wayside2_grass_resource_names)) {
            tconfig.near_wayside2_grass_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::near_wayside2_grass_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_rocks_resource_names)) {
            tconfig.near_grass_terrain_style_config.near_resource_names_mountain_regular = args.arguments.children(KnownArgs::near_rocks_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_wayside1_rocks_resource_names)) {
            tconfig.near_wayside1_grass_terrain_style_config.near_resource_names_mountain_regular = args.arguments.children(KnownArgs::near_wayside1_rocks_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_wayside2_rocks_resource_names)) {
            tconfig.near_wayside2_grass_terrain_style_config.near_resource_names_mountain_regular = args.arguments.children(KnownArgs::near_wayside2_rocks_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_flowers_resource_names)) {
            tconfig.near_flowers_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::near_flowers_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::far_flowers_resource_names)) {
            tconfig.far_flowers_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::far_flowers_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::near_trees_resource_names)) {
            tconfig.near_trees_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::near_trees_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::far_trees_resource_names)) {
            tconfig.far_trees_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::far_trees_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::dirt_decals_resource_names)) {
            tconfig.no_grass_decals_terrain_style_config.near_resource_names_valley_regular = args.arguments.children(KnownArgs::dirt_decals_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains_non_null(KnownArgs::wayside_resource_names)) {
            config.waysides = args.arguments.children(KnownArgs::wayside_resource_names, [&parse_resource_name_func](const JsonMacroArguments& a){
                a.validate(WaysideKnownArgs::options, "wayside: ");
                return WaysideResourceNames{
                    .min_dist = a.at<float>(WaysideKnownArgs::min_dist),
                    .max_dist = a.at<float>(WaysideKnownArgs::max_dist),
                    .resource_names = a.children(WaysideKnownArgs::resource_names, parse_resource_name_func)
                };
            });
        }
        if (args.arguments.contains(KnownArgs::extrusion_ambient_occlusion)) {
            config.extrusion_ambient_occlusion = args.arguments.at<float>(KnownArgs::extrusion_ambient_occlusion);
        }
        if (args.arguments.contains(KnownArgs::laplace_ambient_occlusion)) {
            config.laplace_ambient_occlusion = args.arguments.at<float>(KnownArgs::laplace_ambient_occlusion);
        }
        if (args.arguments.contains(KnownArgs::bounding_terrain_type)) {
            config.bounding_terrain_type = terrain_type_from_string(args.arguments.at<std::string>(KnownArgs::bounding_terrain_type));
        }
        if (args.arguments.contains(KnownArgs::default_terrain_type)) {
            config.default_terrain_type = terrain_type_from_string(args.arguments.at<std::string>(KnownArgs::default_terrain_type));
        }
        if (args.arguments.contains(KnownArgs::default_street_width)) {
            config.default_street_width = args.arguments.at<float>(KnownArgs::default_street_width);
        }
        if (args.arguments.contains(KnownArgs::default_lane_width)) {
            config.default_lane_width = args.arguments.at<float>(KnownArgs::default_lane_width);
        }
        if (args.arguments.contains(KnownArgs::default_tunnel_pipe_width)) {
            config.default_tunnel_pipe_width = args.arguments.at<float>(KnownArgs::default_tunnel_pipe_width);
        }
        if (args.arguments.contains(KnownArgs::default_tunnel_pipe_height)) {
            config.default_tunnel_pipe_height = args.arguments.at<float>(KnownArgs::default_tunnel_pipe_height);
        }
        if (args.arguments.contains(KnownArgs::scale)) {
            config.scale = args.arguments.at<float>(KnownArgs::scale);
        }
        if (args.arguments.contains(KnownArgs::height_scale)) {
            config.height_scale = args.arguments.at<float>(KnownArgs::height_scale);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_terrain)) {
            config.uv_scale_terrain = args.arguments.at<float>(KnownArgs::uv_scale_terrain);
        }
        if (args.arguments.contains(KnownArgs::uv_period_terrain)) {
            config.uv_period_terrain = args.arguments.at<float>(KnownArgs::uv_period_terrain);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_street)) {
            config.uv_scales_street[RoadType::STREET] = args.arguments.at<float>(KnownArgs::uv_scale_street);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_path)) {
            config.uv_scales_street[RoadType::PATH] = args.arguments.at<float>(KnownArgs::uv_scale_path);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_wall)) {
            config.uv_scales_street[RoadType::WALL] = args.arguments.at<float>(KnownArgs::uv_scale_wall);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_facade)) {
            config.uv_scale_facade = args.arguments.at<float>(KnownArgs::uv_scale_facade);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_ceiling)) {
            config.uv_scale_ceiling = args.arguments.at<float>(KnownArgs::uv_scale_ceiling);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_barrier_wall)) {
            config.uv_scale_barrier_wall = args.arguments.at<float>(KnownArgs::uv_scale_barrier_wall);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_highway_wall)) {
            config.uv_scale_highway_wall = args.arguments.at<float>(KnownArgs::uv_scale_highway_wall);
        }
        if (args.arguments.contains(KnownArgs::uv_scale_crossings)) {
            config.uv_scale_crossings = args.arguments.at<float>(KnownArgs::uv_scale_crossings);
        }
        if (args.arguments.contains(KnownArgs::boundary_barrier_height)) {
            config.boundary_barrier_height = args.arguments.at<float>(KnownArgs::boundary_barrier_height);
        }
        if (args.arguments.contains(KnownArgs::boundary_barrier_style)) {
            config.boundary_barrier_style = args.arguments.at<std::string>(KnownArgs::boundary_barrier_style);
        }
        if (args.arguments.contains(KnownArgs::with_roofs)) {
            config.with_roofs = args.arguments.at<bool>(KnownArgs::with_roofs);
        }
        if (args.arguments.contains(KnownArgs::with_ceilings)) {
            config.with_ceilings = args.arguments.at<bool>(KnownArgs::with_ceilings);
        }
        if (args.arguments.contains(KnownArgs::building_bottom)) {
            config.building_bottom = args.arguments.at<float>(KnownArgs::building_bottom);
        }
        if (args.arguments.contains(KnownArgs::default_building_top)) {
            config.default_building_top = args.arguments.at<float>(KnownArgs::default_building_top);
        }
        if (args.arguments.contains(KnownArgs::default_barrier_top)) {
            config.default_barrier_top = args.arguments.at<float>(KnownArgs::default_barrier_top);
        }
        if (args.arguments.contains(KnownArgs::remove_backfacing_triangles)) {
            config.remove_backfacing_triangles = args.arguments.at<bool>(KnownArgs::remove_backfacing_triangles);
        }
        if (args.arguments.contains(KnownArgs::with_tree_nodes)) {
            config.with_tree_nodes = args.arguments.at<bool>(KnownArgs::with_tree_nodes);
        }
        if (args.arguments.contains(KnownArgs::forest_outline_tree_distance)) {
            config.forest_outline_tree_distance = args.arguments.at<float>(KnownArgs::forest_outline_tree_distance);
        }
        if (args.arguments.contains(KnownArgs::forest_outline_tree_inwards_distance)) {
            config.forest_outline_tree_inwards_distance = args.arguments.at<float>(KnownArgs::forest_outline_tree_inwards_distance);
        }
        if (args.arguments.contains(KnownArgs::much_grass_distance)) {
            config.much_grass_distance = args.arguments.at<float>(KnownArgs::much_grass_distance);
        }
        if (args.arguments.contains(KnownArgs::much_near_grass_distance)) {
            tconfig.near_grass_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_near_grass_distance);
        }
        if (args.arguments.contains(KnownArgs::much_far_grass_distance)) {
            tconfig.far_grass_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_far_grass_distance);
        }
        if (args.arguments.contains(KnownArgs::much_near_wayside1_grass_distance)) {
            tconfig.near_wayside1_grass_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_near_wayside1_grass_distance);
        }
        if (args.arguments.contains(KnownArgs::much_near_wayside2_grass_distance)) {
            tconfig.near_wayside2_grass_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_near_wayside2_grass_distance);
        }
        if (args.arguments.contains(KnownArgs::much_near_flowers_distance)) {
            tconfig.near_flowers_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_near_flowers_distance);
        }
        if (args.arguments.contains(KnownArgs::much_far_flowers_distance)) {
            tconfig.far_flowers_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_far_flowers_distance);
        }
        if (args.arguments.contains(KnownArgs::much_near_trees_distance)) {
            tconfig.near_trees_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_near_trees_distance);
        }
        if (args.arguments.contains(KnownArgs::much_far_trees_distance)) {
            tconfig.far_trees_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::much_far_trees_distance);
        }
        if (args.arguments.contains(KnownArgs::dirt_decals_distance)) {
            tconfig.no_grass_decals_terrain_style_config.much_near_distance = args.arguments.at<float>(KnownArgs::dirt_decals_distance);
        }
        if (args.arguments.contains(KnownArgs::raceway_beacon_distance)) {
            config.raceway_beacon_distance = args.arguments.at<float>(KnownArgs::raceway_beacon_distance);
        }
        if (args.arguments.contains(KnownArgs::with_terrain)) {
            config.with_terrain = args.arguments.at<bool>(KnownArgs::with_terrain);
        }
        if (args.arguments.contains(KnownArgs::with_buildings)) {
            config.with_buildings = args.arguments.at<bool>(KnownArgs::with_buildings);
        }
        if (args.arguments.contains(KnownArgs::only_raceways_and_walls)) {
            config.only_raceways_and_walls = args.arguments.at<bool>(KnownArgs::only_raceways_and_walls);
        }
        if (args.arguments.contains(KnownArgs::with_street_way_points)) {
            config.with_street_way_points = args.arguments.at<bool>(KnownArgs::with_street_way_points);
        }
        if (args.arguments.contains(KnownArgs::with_sidewalk_way_points)) {
            config.with_sidewalk_way_points = args.arguments.at<bool>(KnownArgs::with_sidewalk_way_points);
        }
        if (args.arguments.contains(KnownArgs::highway_name_pattern)) {
            config.highway_name_pattern = args.arguments.at<std::string>(KnownArgs::highway_name_pattern);
        }
        if (args.arguments.contains_non_null(KnownArgs::excluded_highways)) {
            config.excluded_highways = args.arguments.at<std::set<std::string>>(KnownArgs::excluded_highways);
        }
        if (args.arguments.contains_non_null(KnownArgs::path_tags)) {
            config.path_tags = args.arguments.at<std::set<std::string>>(KnownArgs::path_tags);
        }
        if (args.arguments.contains_non_null(KnownArgs::smoothed_highways)) {
            config.smoothed_highways = args.arguments.at<std::set<std::string>>(KnownArgs::smoothed_highways);
        }
        if (args.arguments.contains(KnownArgs::max_smooth_highway_length)) {
            config.max_smooth_highway_length = args.arguments.at<float>(KnownArgs::max_smooth_highway_length) * meters;
        }
        if (args.arguments.contains(KnownArgs::steiner_point_distances_road)) {
            config.steiner_point_distances_road = args.arguments.at_vector_non_null<double>(KnownArgs::steiner_point_distances_road, from_meters);
        }
        if (args.arguments.contains(KnownArgs::steiner_point_distances_steiner)) {
            config.steiner_point_distances_steiner = args.arguments.at_vector_non_null<double>(KnownArgs::steiner_point_distances_steiner, from_meters);
        }
        if (args.arguments.contains(KnownArgs::min_dist_to_terrain_region)) {
            config.min_dist_to_terrain_region = args.arguments.at<float>(KnownArgs::min_dist_to_terrain_region);
        }
        if (args.arguments.contains(KnownArgs::curb_alpha)) {
            config.curb_alpha = args.arguments.at<float>(KnownArgs::curb_alpha);
        }
        if (args.arguments.contains(KnownArgs::curb2_alpha)) {
            config.curb2_alpha = args.arguments.at<float>(KnownArgs::curb2_alpha);
        }
        if (args.arguments.contains(KnownArgs::curb_uv_x)) {
            config.curb_uv(0) = args.arguments.at<float>(KnownArgs::curb_uv_x);
        }
        if (args.arguments.contains(KnownArgs::curb_uv_y)) {
            config.curb_uv(1) = args.arguments.at<float>(KnownArgs::curb_uv_y);
        }
        if (args.arguments.contains(KnownArgs::curb2_uv_x)) {
            config.curb2_uv(0) = args.arguments.at<float>(KnownArgs::curb2_uv_x);
        }
        if (args.arguments.contains(KnownArgs::curb2_uv_y)) {
            config.curb2_uv(1) = args.arguments.at<float>(KnownArgs::curb2_uv_y);
        }
        if (args.arguments.contains(KnownArgs::curb_color)) {
            config.curb_color = args.arguments.at<FixedArray<float, 3>>(KnownArgs::curb_color);
        }
        if (args.arguments.contains(KnownArgs::racing_line_width_x)) {
            config.racing_line_width_x = args.arguments.at<float>(KnownArgs::racing_line_width_x);
        }
        if (args.arguments.contains(KnownArgs::racing_line_scale_y)) {
            config.racing_line_scale_y = args.arguments.at<float>(KnownArgs::racing_line_scale_y);
        }
        if (args.arguments.contains(KnownArgs::raise_streets_amount)) {
            config.raise_streets_amount = args.arguments.at<float>(KnownArgs::raise_streets_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_curb_amount)) {
            config.extrude_curb_amount = args.arguments.at<float>(KnownArgs::extrude_curb_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_street_amount)) {
            config.extrude_street_amount = args.arguments.at<float>(KnownArgs::extrude_street_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_air_curb_amount)) {
            config.extrude_air_curb_amount = args.arguments.at<float>(KnownArgs::extrude_air_curb_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_air_support_amount)) {
            config.extrude_air_support_amount = args.arguments.at<float>(KnownArgs::extrude_air_support_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_wall_amount)) {
            config.extrude_wall_amount = args.arguments.at<float>(KnownArgs::extrude_wall_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_grass_amount)) {
            config.extrude_grass_amount = args.arguments.at<float>(KnownArgs::extrude_grass_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_elevated_grass_amount)) {
            config.extrude_elevated_grass_amount = args.arguments.at<float>(KnownArgs::extrude_elevated_grass_amount);
        }
        if (args.arguments.contains(KnownArgs::extrude_water_floor_amout)) {
            config.extrude_water_floor_amout = args.arguments.at<float>(KnownArgs::extrude_water_floor_amout);
        }
        if (args.arguments.contains_non_null(KnownArgs::street_light_resource_names)) {
            config.street_light_resource_names = args.arguments.children(KnownArgs::street_light_resource_names, parse_resource_name_func);
        }
        if (args.arguments.contains(KnownArgs::max_wall_width)) {
            config.max_wall_width = args.arguments.at<float>(KnownArgs::max_wall_width);
        }
        if (args.arguments.contains(KnownArgs::with_height_bindings)) {
            config.with_height_bindings = args.arguments.at<bool>(KnownArgs::with_height_bindings);
        }
        if (args.arguments.contains(KnownArgs::street_node_smoothness)) {
            float v = args.arguments.at<float>(KnownArgs::street_node_smoothness);
            config.street_node_smoothness = 1.f - (float)std::pow(10, -v);
        }
        if (args.arguments.contains(KnownArgs::street_node_smoothing_iterations)) {
            config.street_node_smoothing_iterations = args.arguments.at<size_t>(KnownArgs::street_node_smoothing_iterations);
        }
        if (args.arguments.contains(KnownArgs::street_edge_smoothness)) {
            config.street_edge_smoothness = args.arguments.at<float>(KnownArgs::street_edge_smoothness);
        }
        if (args.arguments.contains(KnownArgs::terrain_edge_smoothness)) {
            config.terrain_edge_smoothness = args.arguments.at<float>(KnownArgs::terrain_edge_smoothness);
        }
        if (args.arguments.contains(KnownArgs::bump_height)) {
            config.bump_height = args.arguments.at<float>(KnownArgs::bump_height);
        }
        if (args.arguments.contains(KnownArgs::driving_direction)) {
            config.driving_direction = driving_direction_from_string(args.arguments.at<std::string>(KnownArgs::driving_direction));
        }
        if (args.arguments.contains(KnownArgs::blend_street)) {
            config.blend_street = args.arguments.at<bool>(KnownArgs::blend_street);
        }
        if (args.arguments.contains(KnownArgs::layer_heights_layer)) {
            layer_heights_layer = args.arguments.at<std::vector<double>>(KnownArgs::layer_heights_layer);
        }
        if (args.arguments.contains(KnownArgs::layer_heights_height)) {
            layer_heights_height = args.arguments.at<std::vector<double>>(KnownArgs::layer_heights_height);
        }
        if (args.arguments.contains(KnownArgs::game_level)) {
            config.game_level = args.arguments.at<std::string>(KnownArgs::game_level);
        }
        if (args.arguments.contains(KnownArgs::base_osm_map_resource)) {
            config.base_osm_map_resource = args.arguments.at<std::string>(KnownArgs::base_osm_map_resource);
        }
        if (args.arguments.contains(KnownArgs::navmesh_resource)) {
            config.navmesh_resource = args.arguments.at<std::string>(KnownArgs::navmesh_resource);
        }
        if (args.arguments.contains(KnownArgs::agent_radius)) {
            config.agent_radius = args.arguments.at<float>(KnownArgs::agent_radius);
        }
        if (args.arguments.contains(KnownArgs::refine_explicit_waypoints)) {
            config.refine_explicit_waypoints = args.arguments.at<bool>(KnownArgs::refine_explicit_waypoints);
        }
    };
    if (resource_name.empty()) {
        THROW_OR_ABORT("Osm resource name not set");
    }
    config.layer_heights = Interp<double>(
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
        std::scoped_lock lock{cache_file_mutex};
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
            scene_node_resources,
            cache_filename,
            resource_name);
    } else {
        osm_map_resource = std::make_shared<OsmMapResource>(
            scene_node_resources,
            config,
            resource_name);
        if (enable_cache) {
            osm_map_resource->save_to_file(cache_filename);
            if (old_cache_file_version != CACHE_FILE_VERSION) {
                std::scoped_lock lock{cache_file_mutex};
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
    scene_node_resources.add_resource(resource_name, osm_map_resource);
};
