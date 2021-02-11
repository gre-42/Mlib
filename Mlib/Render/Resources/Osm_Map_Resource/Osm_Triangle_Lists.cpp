#include "Osm_Triangle_Lists.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>

using namespace Mlib;

OsmTriangleLists::OsmTriangleLists(const OsmResourceConfig& config)
{
    tl_terrain = std::make_shared<TriangleList>("terrain", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_terrain_visuals = std::make_shared<TriangleList>("tl_terrain_visuals", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .collide = false,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_terrain_street_extrusion = std::make_shared<TriangleList>("terrain_street_extrusion", Material{
        .dirt_texture = config.dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
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
        .textures = {primary_rendering_resources->get_blend_map_texture(config.street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .blend_mode = config.blend_street ? BlendMode::CONTINUOUS : BlendMode::OFF,
        .depth_func_equal = config.blend_street,
        .draw_distance_noperations = 1000}.compute_color_mode()); // mixed_texture: terrain_texture
    tl_path = std::make_shared<TriangleList>("path", Material{
        .continuous_blending_z_order = config.blend_street ? 1 : 0,
        .textures = {primary_rendering_resources->get_blend_map_texture(config.path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .blend_mode = config.blend_street ? BlendMode::CONTINUOUS : BlendMode::OFF,
        .depth_func_equal = config.blend_street,
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
}
