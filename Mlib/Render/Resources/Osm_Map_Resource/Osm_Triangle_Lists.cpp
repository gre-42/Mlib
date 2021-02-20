#include "Osm_Triangle_Lists.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>

using namespace Mlib;

OsmTriangleLists::OsmTriangleLists(
    const OsmResourceConfig& config,
    const Material& tunnel_pipe_material)
{
    tl_terrain = std::make_shared<TriangleList>("terrain", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_terrain_visuals = std::make_shared<TriangleList>("tl_terrain_visuals", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .collide = false,
        .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_terrain_street_extrusion = std::make_shared<TriangleList>("terrain_street_extrusion", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());

    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    for (auto& t : config.terrain_textures) {
        // BlendMapTexture bt{ .texture_descriptor = {.color = t, .normal = primary_rendering_resources->get_normalmap(t), .anisotropic_filtering_level = anisotropic_filtering_level } };
        BlendMapTexture bt = primary_rendering_resources->get_blend_map_texture(t);
        tl_terrain->material_.textures.push_back(bt);
        tl_terrain_visuals->material_.textures.push_back(bt);
        tl_terrain_street_extrusion->material_.textures.push_back(bt);
    }
    tl_terrain->material_.compute_color_mode();
    tl_terrain_visuals->material_.compute_color_mode();
    tl_terrain_street_extrusion->material_.compute_color_mode();
    tl_street_crossing = std::make_shared<TriangleList>("street_crossing", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.street_crossing_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_path_crossing = std::make_shared<TriangleList>("path_crossing", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.path_crossing_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_street = std::make_shared<TriangleList>("street", Material{
        .continuous_blending_z_order = config.blend_street ? 1 : 0,
        .blend_mode = config.blend_street ? BlendMode::CONTINUOUS : BlendMode::OFF,
        .textures = {primary_rendering_resources->get_blend_map_texture(config.street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .depth_func_equal = config.blend_street,
        .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_path = std::make_shared<TriangleList>("path", Material{
        .continuous_blending_z_order = config.blend_street ? 1 : 0,
        .blend_mode = config.blend_street ? BlendMode::CONTINUOUS : BlendMode::OFF,
        .textures = {primary_rendering_resources->get_blend_map_texture(config.path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .depth_func_equal = config.blend_street,
        .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    WrapMode curb_wrap_mode_s = (config.extrude_curb_amount != 0) || ((config.curb_alpha != 1) && (config.extrude_street_amount != 0)) ? WrapMode::REPEAT : WrapMode::CLAMP_TO_EDGE;
    tl_curb_street = std::make_shared<TriangleList>("curb_street", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.curb_street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = curb_wrap_mode_s,
        .specularity = OrderableFixedArray{fixed_full<float, 3>((float)(config.extrude_curb_amount == 0))},
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_curb_path = std::make_shared<TriangleList>("curb_path", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.curb_path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = curb_wrap_mode_s,
        .specularity = OrderableFixedArray{fixed_full<float, 3>((float)(config.extrude_curb_amount == 0))},
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_curb2_street = std::make_shared<TriangleList>("curb_street", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.curb2_street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_curb2_path = std::make_shared<TriangleList>("curb_path", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.curb2_path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_air_curb_street = std::make_shared<TriangleList>("air_curb_street", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.air_curb_street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = curb_wrap_mode_s,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_air_curb_path = std::make_shared<TriangleList>("air_curb_path", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.air_curb_path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = curb_wrap_mode_s,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_air_support = std::make_shared<TriangleList>("air_support", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.air_support_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = curb_wrap_mode_s,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_tunnel_pipe = std::make_shared<TriangleList>("tunnel_pipe", Material{
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .ambience = tunnel_pipe_material.ambience,
        .diffusivity = tunnel_pipe_material.diffusivity,
        .specularity = tunnel_pipe_material.specularity,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_tunnel_bdry = std::make_shared<TriangleList>("tunnel_bdry", Material());
}

OsmTriangleLists::~OsmTriangleLists()
{}

#define INSERT(a) a->triangles_.insert(a->triangles_.end(), other.a->triangles_.begin(), other.a->triangles_.end())
void OsmTriangleLists::insert(const OsmTriangleLists& other) {
    INSERT(tl_terrain);
    INSERT(tl_terrain_visuals);
    INSERT(tl_terrain_street_extrusion);
    INSERT(tl_street_crossing);
    INSERT(tl_path_crossing);
    INSERT(tl_street);
    INSERT(tl_path);
    INSERT(tl_curb_street);
    INSERT(tl_curb_path);
    INSERT(tl_curb2_street);
    INSERT(tl_curb2_path);
    INSERT(tl_air_curb_street);
    INSERT(tl_air_curb_path);
    INSERT(tl_air_support);
    INSERT(tl_tunnel_pipe);
}
#undef INSERT

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_street_wo_curb() const {
    return std::list<std::shared_ptr<TriangleList>>{
        tl_street_crossing,
        tl_path_crossing,
        tl_street,
        tl_path};
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_street() const {
    return std::list<std::shared_ptr<TriangleList>>{
        tl_street_crossing,
        tl_path_crossing,
        tl_street,
        tl_path,
        tl_curb_street,
        tl_curb_path,
        tl_curb2_street,
        tl_curb2_path,
        tl_air_curb_street,
        tl_air_curb_path,
        tl_air_support};
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_flat() const {
    return std::list<std::shared_ptr<TriangleList>>{
        tl_terrain,
        tl_terrain_visuals,
        tl_terrain_street_extrusion,
        tl_street_crossing,
        tl_path_crossing,
        tl_street,
        tl_path,
        tl_curb_street,
        tl_curb_path,
        tl_curb2_street,
        tl_curb2_path,
        tl_air_curb_street,
        tl_air_curb_path,
        tl_air_support};
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_all() const {
    return std::list<std::shared_ptr<TriangleList>>{
        tl_terrain,
        tl_terrain_visuals,
        tl_terrain_street_extrusion,
        tl_street_crossing,
        tl_path_crossing,
        tl_street,
        tl_path,
        tl_curb_street,
        tl_curb_path,
        tl_curb2_street,
        tl_curb2_path,
        tl_air_curb_street,
        tl_air_curb_path,
        tl_air_support,
        tl_tunnel_pipe};
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_with_vertex_normals() const {
    return std::list<std::shared_ptr<TriangleList>>{
        tl_terrain,
        tl_terrain_visuals,
        tl_street_crossing,
        tl_path_crossing,
        tl_street,
        tl_path};
}
