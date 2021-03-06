#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Map.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Style.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <cmath>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

enum class BlendMode;
enum class DrivingDirection;
struct WaysideResourceNames;
struct RoadProperties;
enum class TerrainType;
enum class WrapMode;

struct RoadStyle {
    std::vector<std::string> textures;
    float uvx;
};

struct BarrierStyle {
    std::string texture;
    FixedArray<float, 2> uv;
    BlendMode blend_mode;
    WrapMode wrap_mode_t;
    bool reorient_uv0;
};

struct OsmResourceConfig {
    OsmResourceConfig(const OsmResourceConfig&) = delete;
    OsmResourceConfig& operator = (const OsmResourceConfig&) = delete;
    OsmResourceConfig();
    ~OsmResourceConfig();
    std::string filename;
    std::string heightmap;
    std::string heightmap_mask;
    size_t heightmap_extension = 0;
    std::map<TerrainType, std::vector<std::string>> terrain_textures;
    std::map<TerrainType, std::string> terrain_dirt_textures;
    std::string street_dirt_texture;
    std::string street_reflection_map;
    std::map<RoadProperties, RoadStyle> street_texture;
    std::map<RoadType, std::string> street_crossing_texture;
    std::map<RoadType, std::string> curb_street_texture;
    std::map<RoadType, std::string> curb2_street_texture;
    std::map<RoadType, std::string> air_curb_street_texture;
    float racing_line_width_x = 3.0f;
    float racing_line_scale_y = 0.5f;
    std::string racing_line_texture;
    std::string racing_line_track;
    std::string racing_line_playback;
    std::string air_support_texture;
    std::vector<std::string> socle_textures;
    float ambient_occlusion = 0.5f;
    Interp<float, FixedArray<float, 3>> height_colors{
        std::vector<float>{0.f, 15.f},
        std::vector<FixedArray<float, 3>>{
            FixedArray<float, 3>{ 1.f, 1.f, 1.f },
            FixedArray<float, 3>{ 0.8f, 0.8f, 0.8f }},
        OutOfRangeBehavior::CLAMP};
    std::vector<FacadeTexture> facade_textures;
    std::string ceiling_texture;
    std::map<std::string, BarrierStyle> barrier_styles;
    std::string tunnel_pipe_texture;
    std::string tunnel_pipe_resource_name = "pipe_box";
    std::string tunnel_bdry_resource_name = "pipe_box_boundary";
    Map<RoadType, std::string> street_surface_central_resource_names;
    Map<RoadType, std::string> street_surface_endpoint0_resource_names;
    Map<RoadType, std::string> street_surface_endpoint1_resource_names;
    Map<RoadType, std::string> street_bumps_central_resource_names;
    Map<RoadType, std::string> street_bumps_endpoint0_resource_names;
    Map<RoadType, std::string> street_bumps_endpoint1_resource_names;
    std::string water_texture;
    float water_height = 0;
    std::string roof_texture;
    std::vector<ParsedResourceName> tree_resource_names;
    std::vector<ParsedResourceName> grass_resource_names;
    TerrainStyleConfig near_grass_terrain_style_config{ .much_near_distance = 2 };
    TerrainStyleConfig near_flowers_terrain_style_config{ .much_near_distance = 2 };
    TerrainStyleConfig no_grass_decals_terrain_style_config{ .much_near_distance = 10 };
    std::list<WaysideResourceNames> waysides;
    TerrainType bounding_terrain_type = TerrainType::UNDEFINED;
    TerrainType default_terrain_type = TerrainType::UNDEFINED;
    float default_street_width = 7;
    float default_lane_width = 4;
    float default_tunnel_pipe_width = 1.f;
    float default_tunnel_pipe_height = 4.f;
    float roof_width = 2;
    float scale = 1;
    float uv_scale_terrain = 1;
    float uv_period_terrain = 1;
    std::map<RoadType, float> uv_scales_street;
    float uv_scale_grass = 1;
    float uv_scale_crossings = 1.f;
    float uv_scale_facade = 1;
    float uv_scale_ceiling = 1;
    float uv_scale_barrier_wall = 1;
    float uv_scale_highway_wall = 1;
    bool with_roofs = true;
    bool with_ceilings = false;
    float building_bottom = -3;
    float default_building_top = 6;
    float default_barrier_top = 3;
    bool remove_backfacing_triangles = true;
    bool with_tree_nodes = true;
    float forest_outline_tree_distance = 0.15f;
    float forest_outline_tree_inwards_distance = 0;
    float much_grass_distance = 5;
    float raceway_beacon_distance = INFINITY;
    float min_dist_to_road = 0.5f;
    bool with_terrain = true;
    bool with_buildings = true;
    bool only_raceways_and_walls = false;
    bool with_street_way_points = true;
    bool with_sidewalk_way_points = true;
    std::string highway_name_pattern = "";
    std::set<std::string> excluded_highways = { "pedestrian", "path", "footway", "cycleway", "steps" };
    std::set<std::string> path_tags = { "track", "tertiary" };
    std::vector<double> steiner_point_distances_road = { 100. };
    std::vector<double> steiner_point_distances_steiner = { 100. };
    float curb_alpha = 0.9f;
    float curb2_alpha = 0.95f;
    FixedArray<float, 2> curb_uv = FixedArray<float, 2>{1.f, 1.f};
    FixedArray<float, 2> curb2_uv = FixedArray<float, 2>{1.f, 1.f};
    FixedArray<float, 3> curb_color = FixedArray<float, 3>{ 1.f, 1.f, 1.f };
    float raise_streets_amount = 0.2f;
    float extrude_curb_amount = 0;
    float extrude_street_amount = 0;
    float extrude_air_curb_amount = NAN;
    float extrude_air_support_amount = 0;
    float extrude_wall_amount = 0;
    float extrude_grass_amount = 0;
    float extrude_elevated_grass_amount = 0;
    float extrude_water_floor_amout = 0;
    std::vector<ParsedResourceName> street_light_resource_names = {};
    float max_wall_width = 5;
    bool with_height_bindings = false;
    float street_node_smoothness = 0;
    float street_edge_smoothness = 0;
    float terrain_edge_smoothness = 0;
    float bump_height = 1.f;
    DrivingDirection driving_direction = DrivingDirection::CENTER;
    bool blend_street = false;
    Interp<float> layer_heights{std::vector<float>{}, std::vector<float>{}};
    std::string game_level;
    std::string base_osm_map_resource;
    std::string navmesh_resource;
    float agent_radius = 0.6f;
};

}
