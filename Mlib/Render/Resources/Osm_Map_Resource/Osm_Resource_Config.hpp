#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
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
template <class TData>
class Interp;
enum class RoadType;
struct RoadProperties;

struct RoadStyle {
    std::string texture;
    float uvx;
};

struct OsmResourceConfig {
    OsmResourceConfig();
    ~OsmResourceConfig();
    std::string filename;
    std::string heightmap;
    std::vector<std::string> terrain_textures;
    std::string dirt_texture;
    std::map<RoadProperties, RoadStyle> street_texture;
    std::map<RoadType, std::string> street_crossing_texture;
    std::map<RoadType, std::string> curb_street_texture;
    std::map<RoadType, std::string> curb2_street_texture;
    std::map<RoadType, std::string> air_curb_street_texture;
    std::string air_support_texture;
    std::vector<std::string> facade_textures;
    std::string ceiling_texture;
    std::string barrier_texture;
    std::string tunnel_pipe_texture;
    std::string tunnel_pipe_resource_name = "pipe_box";
    std::string tunnel_bdry_resource_name = "pipe_box_boundary";
    std::string water_texture;
    float water_height = 0;
    BlendMode barrier_blend_mode;
    std::string roof_texture;
    std::vector<std::string> tree_resource_names;
    std::vector<std::string> grass_resource_names;
    std::vector<std::string> near_grass_resource_names;
    std::list<WaysideResourceNames> waysides;
    float default_street_width = 7;
    float default_lane_width = 4;
    float default_tunnel_pipe_width = 1.f;
    float default_tunnel_pipe_height = 4.f;
    float roof_width = 2;
    float scale = 1;
    float uv_scale_terrain = 1;
    float uv_scale_street = 1;
    float uv_scale_facade = 1;
    float uv_scale_ceiling = 1;
    float uv_scale_barrier_wall = 1;
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
    float much_near_grass_distance = 2;
    float raceway_beacon_distance = INFINITY;
    bool with_terrain = true;
    bool with_buildings = true;
    bool only_raceways = false;
    std::string highway_name_pattern = "";
    std::set<std::string> excluded_highways = { "pedestrian", "path", "footway", "cycleway", "steps" };
    std::set<std::string> path_tags = { "track", "tertiary" };
    std::vector<float> steiner_point_distances_road = { 100.f };
    std::vector<float> steiner_point_distances_steiner = { 100.f };
    float curb_alpha = 0.9f;
    float curb2_alpha = 0.95f;
    float curb_uv_x = 1;
    float curb2_uv_x = 1;
    FixedArray<float, 3> curb_color = FixedArray<float, 3>{ 1.f, 1.f, 1.f };
    float raise_streets_amount = 0.2f;
    float extrude_curb_amount = 0;
    float extrude_street_amount = 0;
    float extrude_air_curb_amount = NAN;
    float extrude_air_support_amount = 0;
    std::vector<std::string> street_light_resource_names = {};
    float max_wall_width = 5;
    bool with_height_bindings = false;
    float street_node_smoothness = 0;
    float street_edge_smoothness = 0;
    float terrain_edge_smoothness = 0;
    DrivingDirection driving_direction = DrivingDirection::CENTER;
    bool blend_street = false;
    Interp<float> layer_heights{std::vector<float>{}, std::vector<float>{}};
};

}
