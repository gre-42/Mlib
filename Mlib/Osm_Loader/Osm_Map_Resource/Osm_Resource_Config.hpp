#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Geometry/Mesh/Contour_Detection_Strategy.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Barrier_Style.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Socle_Texture.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertical_Subdivision.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Renderables/Color_And_Probability.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Triangle_Sampler_Resource_Config.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <list>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

enum class BlendMode;
enum class DrivingDirection;
struct WaysideResourceNamesSurface;
struct WaysideResourceNamesVertex;
struct RoadProperties;
enum class TerrainType;
enum class WrapMode;
enum class PhysicsMaterial: uint32_t;

struct RoadStyle {
    std::vector<VariableAndHash<std::string>> textures;
    float uvx;
};

struct CoastConfiguration {
    CompressedScenePos width;
};

void from_json(const nlohmann::json& j, CoastConfiguration& water);

struct WaterTextureConfiguration {
    std::vector<VariableAndHash<std::string>> color;
    std::vector<VariableAndHash<std::string>> alpha;
};

void from_json(const nlohmann::json& j, WaterTextureConfiguration& textures);

struct WaterConfiguration {
    WaterTextureConfiguration textures;
    std::chrono::steady_clock::duration animation_duration;
    AxisAlignedBoundingBox<CompressedScenePos, 2> aabb = uninitialized;
    FixedArray<CompressedScenePos, 2> cell_size = uninitialized;
    CompressedScenePos duplicate_distance;
    UFixedArray<CompressedScenePos, 2> heights;
    CoastConfiguration coast;
    bool generate_tiles;
    bool holes_from_terrain;
    float yangle;
};

void from_json(const nlohmann::json& j, WaterConfiguration& water);

struct OsmResourceConfig {
    OsmResourceConfig(const OsmResourceConfig&) = delete;
    OsmResourceConfig& operator = (const OsmResourceConfig&) = delete;
    OsmResourceConfig();
    ~OsmResourceConfig();
    std::vector<std::string> filenames;
    std::string heightmap;
    std::string heightmap_mask;
    size_t heightmap_extension = 0;
    size_t heightmap_dilation = 0;
    std::string displacementmap;
    double displacementmap_min = 0;
    double displacementmap_uv_scale = 1;
    Interp<float> displacementmap_distance_2_z_scale{ std::vector<float>{}, std::vector<float>{} };
    std::string zonemap;
    double zonemap_width = NAN;
    double zonemap_height = NAN;
    double zonemap_multiplier = 1.;
    float zonemap_jitter = 0.9f;
    double zonemap_step_size = 4.;
    float sparse_triangle_cluster_width = 0.f;
    float dense_triangle_cluster_width = 0.f;
    std::vector<ParsedResourceName> zone_resource_names;
    std::map<TerrainType, PhysicsMaterial> terrain_materials;
    std::map<RoadType, PhysicsMaterial> street_materials;
    std::map<TerrainType, std::vector<VariableAndHash<std::string>>> terrain_textures;
    std::map<TerrainType, VariableAndHash<std::string>> terrain_dirt_textures;
    VariableAndHash<std::string> street_dirt_texture;
    std::map<RoadType, std::vector<VariableAndHash<std::string>>> street_mud_textures;
    std::map<RoadType, VariableAndHash<std::string>> street_reflection_map;
    std::map<TerrainType, VariableAndHash<std::string>> terrain_reflection_map;
    VariableAndHash<std::string> window_reflection_map;
    std::map<RoadType, std::vector<VariableAndHash<std::string>>> street_alpha_textures;
    std::map<RoadType, std::vector<VariableAndHash<std::string>>> street_mud_alpha_textures;
    std::map<RoadProperties, RoadStyle> street_texture;
    std::map<RoadType, std::vector<VariableAndHash<std::string>>> street_crossing_textures;
    std::map<RoadType, VariableAndHash<std::string>> curb_street_texture;
    std::map<RoadType, VariableAndHash<std::string>> curb2_street_texture;
    std::map<RoadType, VariableAndHash<std::string>> air_curb_street_texture;
    float racing_line_width_x = 3.0f;
    float racing_line_scale_y = 0.1f;
    VariableAndHash<std::string> racing_line_texture;
    std::string racing_line_track;
    std::string racing_line_playback;
    VariableAndHash<std::string> air_support_texture;
    std::vector<SocleTexture> socle_textures;
    std::vector<FacadeTexture> entrance_textures;
    float extrusion_ambient_occlusion = 0.5f;
    float laplace_ambient_occlusion = 1.f;
    UUInterp<float, FixedArray<float, 3>> height_colors{
        std::vector<float>{0.f, 15.f},
        UUVector<FixedArray<float, 3>>{
            UFixedArray<float, 3>{ 1.f, 1.f, 1.f },
            UFixedArray<float, 3>{ 0.8f, 0.8f, 0.8f }},
        OutOfRangeBehavior::CLAMP };
    std::vector<FacadeTexture> facade_textures;
    VariableAndHash<std::string> ceiling_texture;
    Map<std::string, BarrierStyle> barrier_styles;
    float boundary_barrier_height = 1.f;
    std::string boundary_barrier_style;
    VariableAndHash<std::string> tunnel_pipe_texture;
    VariableAndHash<std::string> tunnel_pipe_resource_name = VariableAndHash<std::string>{"pipe_box"};
    VariableAndHash<std::string> tunnel_bdry_resource_name = VariableAndHash<std::string>{"pipe_box_boundary"};
    Map<RoadType, VariableAndHash<std::string>> street_surface_central_resource_names;
    Map<RoadType, VariableAndHash<std::string>> street_surface_endpoint0_resource_names;
    Map<RoadType, VariableAndHash<std::string>> street_surface_endpoint1_resource_names;
    Map<RoadType, VariableAndHash<std::string>> street_bumps_central_resource_names;
    Map<RoadType, VariableAndHash<std::string>> street_bumps_endpoint0_resource_names;
    Map<RoadType, VariableAndHash<std::string>> street_bumps_endpoint1_resource_names;
    std::optional<WaterConfiguration> water;
    std::vector<ParsedResourceName> tree_resource_names;
    std::vector<ParsedResourceName> grass_resource_names;
    TriangleSamplerResourceConfig triangle_sampler_resource_config;
    std::vector<WaysideResourceNamesSurface> waysides_surface;
    std::vector<WaysideResourceNamesVertex> waysides_vertex;
    TerrainType bounding_terrain_type = TerrainType::UNDEFINED;
    TerrainType default_terrain_type = TerrainType::UNDEFINED;
    float default_street_width = 7 * meters;
    float default_lane_width = 4 * meters;
    float default_tunnel_pipe_width = 1.f * meters;
    CompressedScenePos default_tunnel_pipe_height = (CompressedScenePos)(4 * meters);
    float scale = 1;
    float triangulation_scale = 1;
    double waypoint_merge_radius = 1 * cm;
    double waypoint_error_radius = 2 * cm;
    CompressedScenePos waypoint_distance = (CompressedScenePos)(2 * meters);
    double height_scale = 1;
    float uv_scale_terrain = 1;
    float uv_period_terrain = 1;
    std::map<RoadType, float> uv_scales_street;
    float uv_scale_grass = 1;
    float uv_scale_crossings = 1.f;
    float uv_scale_facade = 1;
    float uv_scale_roof = 1;
    float uv_scale_ceiling = 1;
    float uv_scale_barrier_wall = 1;
    float uv_scale_highway_wall = 1;
    std::vector<ColorAndProbability> building_colors;
    VariableAndHash<std::string> roof_texture;
    VariableAndHash<std::string> roof_rail_texture;
    float default_roof_9_2_max_building_height = INFINITY * meters;
    std::optional<Roof9_2> default_roof_9_2;
    VariableAndHash<std::string> roof_model;
    VariableAndHash<std::string> bridge_pier_model;
    SceneDir bridge_pier_radius = 1.5f;
    std::vector<VariableAndHash<std::string>> bridge_pier_textures;
    bool with_roofs = true;
    bool with_ceilings = false;
    float building_bottom = -3;
    float default_building_top = 6;
    float default_barrier_top = 3;
    float snap_building_length_ratio = 1.0f;
    float snap_building_length_angle = 10.f * degrees;
    bool default_snap_building_height = false;
    bool default_snap_barrier_height = false;
    float socle_height = 1.2f * meters;
    VerticalSubdivision default_building_vertical_subdivision = VerticalSubdivision::SOCLE;
    bool remove_backfacing_triangles = true;
    bool with_tree_nodes = true;
    float forest_outline_tree_distance = 10.f * meters;
    float forest_outline_tree_inwards_distance = 0 * meters;
    float much_grass_distance = 5.f * meters;
    float raceway_beacon_distance = INFINITY;
    float min_dist_to_road = 0.5f;
    float min_dist_to_terrain_region = 10.f;
    bool with_terrain = true;
    bool with_buildings = true;
    bool only_raceways_and_walls = false;
    bool with_street_way_points = true;
    bool with_sidewalk_way_points = true;
    std::string highway_name_pattern = "";
    std::set<std::string> excluded_highways = { "pedestrian", "path", "footway", "cycleway", "steps" };
    std::set<std::string> path_tags = { "track", "tertiary" };
    std::set<std::string> included_aeroways = { "taxiway", "runway" };
    std::set<std::string> smoothed_highways = {};
    std::set<std::string> smoothed_aeroways = {};
    float max_smooth_highway_length = 5.f;
    std::vector<double> steiner_point_distances_road = { 100. };
    std::vector<double> steiner_point_distances_steiner = { 100. };
    float curb_alpha = 0.9f;
    float curb2_alpha = 0.95f;
    FixedArray<float, 2> curb_uv = FixedArray<float, 2>{ 1.f, 1.f };
    FixedArray<float, 2> curb2_uv = FixedArray<float, 2>{ 1.f, 1.f };
    FixedArray<float, 3> curb_color = FixedArray<float, 3>{ 1.f, 1.f, 1.f };
    float raise_streets_amount = 0.2f;
    CompressedScenePos extrude_curb_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_street_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_air_curb_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_air_support_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_wall_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_grass_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_elevated_grass_amount = (CompressedScenePos)0.f;
    CompressedScenePos extrude_water_floor_amout = (CompressedScenePos)0.f;
    CompressedScenePos building_grass_width = (CompressedScenePos)0.f;
    CompressedScenePos indent_buildings_amount = (CompressedScenePos)0.f;
    std::vector<ParsedResourceName> stop_sign_resource_names = {};
    float max_wall_width = 5;
    bool with_height_bindings = false;
    float street_node_smoothness = 0;
    size_t street_node_smoothing_iterations = 50;
    float street_edge_smoothness = 0;
    float terrain_edge_smoothness = 0;
    Interp<float> terrain_edge_bias{ std::vector<float>{}, std::vector<float>{} };
    float bump_height = 1.f * meters;
    ContourDetectionStrategy contour_detection_strategy = ContourDetectionStrategy::EDGE_NEIGHBOR;
    FixedArray<float, 3> emissive_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> diffuse_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> specular_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> fresnel_ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 2> fog_distances = default_step_distances;
    FixedArray<float, 3> fog_ambient = FixedArray<float, 3>(0.f);
    DrivingDirection driving_direction = DrivingDirection::CENTER;
    std::map<RoadType, bool> blend_street;
    Interp<double> layer_heights{ std::vector<double>{}, std::vector<double>{} };
    bool use_terrain_holes = false;
    float building_cluster_width = 500.f;
    uint32_t max_imposter_texture_size = 0;
    std::string game_level;
    VariableAndHash<std::string> base_osm_map_resource;
    VariableAndHash<std::string> navmesh_resource;
    bool refine_explicit_waypoints = true;
    float agent_radius = 0.6f;
};

}
