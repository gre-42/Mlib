#include "Osm_Triangle_Lists.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Material_Colors.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>

using namespace Mlib;

static PhysicsMaterial physics_material(
    const std::map<TerrainType, PhysicsMaterial>& m,
    TerrainType terrain_type)
{
    auto it = m.find(terrain_type);
    if (it == m.end()) {
        THROW_OR_ABORT("Could not find physics material for terrain type \"" + terrain_type_to_string(terrain_type) + '"');
    }
    return it->second;
}

static Shading terrain_type_specularity(
    const std::map<TerrainType, PhysicsMaterial>& m,
    TerrainType terrain_type,
    const OsmResourceConfig& config)
{
    auto pm = physics_material(m, terrain_type);
    try {
        return material_shading(pm, config);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Error determining specularity for terrain type \"" + to_string(terrain_type) + "\": " + e.what());
    }
}

void RoadPropertiesTriangleList::append(const StyledRoadEntry& entry) {
    lst_.push_back(entry);
}

const StyledRoad& RoadPropertiesTriangleList::operator [] (const RoadProperties& road_properties) const {
    const StyledRoad* result = nullptr;
    for (const auto& l : lst_) {
        if (l.road_properties.type != road_properties.type) {
            continue;
        }
        if (l.road_properties.nlanes <= road_properties.nlanes) {
            result = &l.styled_road;
        }
    }
    if (result == nullptr) {
        THROW_OR_ABORT("Could not find matching triangle list for properties " + (std::string)road_properties);
    }
    return *result;
}

const std::list<StyledRoadEntry>& RoadPropertiesTriangleList::list() const {
    return lst_;
}

template <class EntityType>
void EntityTypeTriangleList<EntityType>::insert(EntityType road_type, const std::shared_ptr<TriangleList<CompressedScenePos>>& lst) {
    if (!lst_.insert({road_type, lst}).second) {
        THROW_OR_ABORT("Could not insert triangle list");
    }
}

template <class EntityType>
bool EntityTypeTriangleList<EntityType>::contains(EntityType road_type) const {
    return lst_.find(road_type) != lst_.end();
}

template <class EntityType>
const std::shared_ptr<TriangleList<CompressedScenePos>>& EntityTypeTriangleList<EntityType>::operator [] (EntityType road_type) const {
    auto it = lst_.find(road_type);
    if (it == lst_.end()) {
        THROW_OR_ABORT("Could not find list with type " + to_string(road_type));
    }
    return it->second;
}

template <class EntityType>
const std::map<EntityType, std::shared_ptr<TriangleList<CompressedScenePos>>>& EntityTypeTriangleList<EntityType>::map() const {
    return lst_;
}

OsmTriangleLists::OsmTriangleLists(
    const OsmResourceConfig& config,
    const std::string& name_suffix)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    tl_terrain = std::make_shared<TerrainTypeTriangleList>();
    // Specify auxiliary triangle lists that are hidden from the results.
    // The mechanism that excludes these lists from the results is that they have
    // PhysicsMaterials::NONE, and tl_terrain-lists with such a material are explicitly
    // excluded from the results.
    tl_terrain->insert(
        TerrainType::STREET_HOLE,
        std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(TerrainType::STREET_HOLE) + name_suffix,
            Material{},
            Morphology{ .physics_material = PhysicsMaterial::NONE }));
    tl_terrain->insert(
        TerrainType::BUILDING_HOLE,
        std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(TerrainType::BUILDING_HOLE) + name_suffix,
            Material{},
            Morphology{ .physics_material = PhysicsMaterial::NONE }));
    tl_terrain->insert(
        TerrainType::OCEAN_GROUND,
        std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(TerrainType::OCEAN_GROUND) + name_suffix,
            Material{},
            Morphology{ .physics_material = PhysicsMaterial::NONE }));
    for (auto& [tt, ttt] : config.terrain_textures) {
        auto dt = config.terrain_dirt_textures.find(tt);
        auto dirt_texture = (dt == config.terrain_dirt_textures.end()) ? VariableAndHash<std::string>{} : dt->second;
        auto rit = config.terrain_reflection_map.find(tt);
        tl_terrain->insert(tt, std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(tt) + name_suffix,
            Material{
                .reflection_map = (rit != config.terrain_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .contains_skidmarks = true,
                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = terrain_type_specularity(config.terrain_materials, tt, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | physics_material(config.terrain_materials, tt) }));
        tl_terrain_visuals.insert(tt, std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(tt) + "_visuals" + name_suffix,
            Material{
                .reflection_map = (rit != config.terrain_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .contains_skidmarks = true,
                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = terrain_type_specularity(config.terrain_materials, tt, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE }));
        tl_terrain_extrusion.insert(tt, std::make_shared<TriangleList<CompressedScenePos>>(
            terrain_type_to_string(tt) + "_street_extrusion" + name_suffix,
            Material{
                .reflection_map = (rit != config.terrain_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .contains_skidmarks = true,
                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = terrain_type_specularity(config.terrain_materials, tt, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | physics_material(config.terrain_materials, tt) }));
        for (auto& t : ttt) {
            // BlendMapTexture bt{ .texture_descriptor = {.color = t, .normal = primary_rendering_resources.get_normalmap(t), .anisotropic_filtering_level = anisotropic_filtering_level } };
            BlendMapTexture bt = primary_rendering_resources.get_blend_map_texture(t);
            (*tl_terrain)[tt]->material.textures_color.push_back(bt);
            tl_terrain_visuals[tt]->material.textures_color.push_back(bt);
            tl_terrain_extrusion[tt]->material.textures_color.push_back(bt);
        }
        (*tl_terrain)[tt]->material.compute_color_mode();
        tl_terrain_visuals[tt]->material.compute_color_mode();
        tl_terrain_extrusion[tt]->material.compute_color_mode();
    }
    for (const auto& [tpe, textures] : config.street_crossing_textures) {
        auto pmit = config.street_materials.find(tpe);
        if (pmit == config.street_materials.end()) {
            THROW_OR_ABORT("Could not find physics material for type \"" + road_type_to_string(tpe) + '"');
        }
        auto rit = config.street_reflection_map.find(tpe);
        std::vector<BlendMapTexture> blend_textures;
        blend_textures.reserve(textures.size());
        for (const VariableAndHash<std::string>& texture : textures) {
            blend_textures.push_back(primary_rendering_resources.get_blend_map_texture(texture));
        }
        tl_street_crossing.insert(tpe, std::make_shared<TriangleList<CompressedScenePos>>(
            "crossing_" + road_type_to_string(tpe) + name_suffix,
            Material{
                .textures_color = blend_textures,
                .reflection_map = (rit != config.street_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .dirt_texture = config.street_dirt_texture,
                .occluded_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::LIGHTMAP_BLOBS,
                .occluder_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .contains_skidmarks = true,
                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = material_shading(pmit->second, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | pmit->second }));
    }
    for (const auto& [road_properties, road_style] : config.street_texture) {
        auto pmit = config.street_materials.find(road_properties.type);
        if (pmit == config.street_materials.end()) {
            THROW_OR_ABORT("Could not find physics material for type \"" + road_type_to_string(road_properties.type) + '"');
        }
        bool blend =
            config.blend_street.contains(road_properties.type) &&
            config.blend_street.at(road_properties.type) &&
            (road_properties.type != RoadType::WALL);
        {
            auto alpha_texture_names = config.street_alpha_textures.contains(road_properties.type)
                ? config.street_alpha_textures.at(road_properties.type)
                : std::vector<VariableAndHash<std::string>>{};
            std::vector<BlendMapTexture> textures_color;
            textures_color.reserve(road_style.textures.size());
            for (const VariableAndHash<std::string>& texture : road_style.textures) {
                textures_color.push_back(primary_rendering_resources.get_blend_map_texture(texture));
            }
            std::vector<BlendMapTexture> textures_alpha;
            textures_alpha.reserve(alpha_texture_names.size());
            for (const VariableAndHash<std::string>& texture : alpha_texture_names) {
                textures_alpha.push_back(primary_rendering_resources.get_blend_map_texture(texture));
            }
            auto rit = config.street_reflection_map.find(road_properties.type);
            tl_street.append(StyledRoadEntry{
                .road_properties = road_properties,
                .styled_road = StyledRoad{
                    .triangle_list = std::make_shared<TriangleList<CompressedScenePos>>(
                        (std::string)road_properties + name_suffix,
                        Material{
                            .blend_mode = blend ? BlendMode::CONTINUOUS : BlendMode::OFF,
                            .continuous_blending_z_order = 1,
                            .depth_func = blend ? DepthFunc::EQUAL : DepthFunc::LESS,
                            .blending_pass = blend ? BlendingPassType::EARLY : BlendingPassType::NONE,
                            .textures_color = textures_color,
                            .textures_alpha = textures_alpha,
                            .reflection_map = (rit != config.street_reflection_map.end())
                                ? rit->second
                                : VariableAndHash<std::string>{},
                            .dirt_texture = config.street_dirt_texture,
                            .occluded_pass = (road_properties.type != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::LIGHTMAP_BLOBS,
                            .occluder_pass = (road_properties.type != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                            .contains_skidmarks = true,
                            // .wrap_mode_s = (road_properties.type != RoadType::WALL) && (road_style.uvx <= 1) ? WrapMode::CLAMP_TO_EDGE : WrapMode::REPEAT,
                            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                            // depth-func==equal requires aggregation, because the terrain is also aggregated.
                            .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                            .shading = material_shading(
                                (road_properties.type != RoadType::WALL) ? pmit->second : PhysicsMaterial::SURFACE_BASE_STONE,
                                config),
                            // .reflect_only_y = true,
                            .draw_distance_noperations = 1000}.compute_color_mode(),
                        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | pmit->second }),
                    .uvx = road_style.uvx}}); // mixed_texture: terrain_texture
        }
        if (blend &&
            !tl_street_mud_visuals.contains(road_properties.type) &&
            config.street_mud_textures.contains(road_properties.type) &&
            (road_properties.type != RoadType::WALL))
        {
            auto alpha_texture_names = config.street_mud_alpha_textures.contains(road_properties.type)
                ? config.street_mud_alpha_textures.at(road_properties.type)
                : std::vector<VariableAndHash<std::string>>{};
            std::vector<BlendMapTexture> textures_color;
            textures_color.reserve(config.street_mud_textures.at(road_properties.type).size());
            for (const VariableAndHash<std::string>& texture : config.street_mud_textures.at(road_properties.type)) {
                textures_color.push_back(primary_rendering_resources.get_blend_map_texture(texture));
            }
            std::vector<BlendMapTexture> textures_alpha;
            textures_alpha.reserve(alpha_texture_names.size());
            for (const VariableAndHash<std::string>& texture : alpha_texture_names) {
                textures_alpha.push_back(primary_rendering_resources.get_blend_map_texture(texture));
            }
            tl_street_mud_visuals.insert(road_properties.type, std::make_shared<TriangleList<CompressedScenePos>>(
                "street_mud_visual_" + road_type_to_string(road_properties.type) + name_suffix,
                Material{
                    .blend_mode = BlendMode::CONTINUOUS,
                    .depth_func = DepthFunc::EQUAL,
                    .blending_pass = BlendingPassType::EARLY,
                    .textures_color = textures_color,
                    .textures_alpha = textures_alpha,
                    .dirt_texture = config.street_dirt_texture,
                    .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                    .occluder_pass = ExternalRenderPassType::NONE,
                    // .wrap_mode_s = (road_style.uvx <= 1) ? WrapMode::CLAMP_TO_EDGE : WrapMode::REPEAT,
                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                    // depth-func==equal requires aggregation, because the terrain is also aggregated.
                    .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                    .shading = material_shading(RawShading::MUD, config),
                    // .reflect_only_y = true,
                    .draw_distance_noperations = 1000}.compute_color_mode(),
                Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE }));
        }
    }
    // WrapMode curb_wrap_mode_s = (config.extrude_curb_amount != 0) || (config.extrude_street_amount != 0)
    //     ? WrapMode::REPEAT
    //     : WrapMode::CLAMP_TO_EDGE;
    for (const auto& [tpe, texture] : config.curb_street_texture) {
        auto pmit = config.street_materials.find(tpe);
        if (pmit == config.street_materials.end()) {
            THROW_OR_ABORT("Could not find physics material for type \"" + road_type_to_string(tpe) + '"');
        }
        auto rit = config.street_reflection_map.find(tpe);
        tl_street_curb.insert(tpe, std::make_shared<TriangleList<CompressedScenePos>>(
            "curb_" + road_type_to_string(tpe) + name_suffix,
            Material{
                .textures_color = {primary_rendering_resources.get_blend_map_texture(texture)},
                .reflection_map = (rit != config.street_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .occluded_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::LIGHTMAP_BLOBS,
                .occluder_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                // .wrap_mode_s = curb_wrap_mode_s,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = material_shading((config.extrude_curb_amount == (CompressedScenePos)0. && tpe != RoadType::WALL) ? RawShading::CURB : RawShading::DEFAULT, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | pmit->second })); // mixed_texture: terrain_texture
    }
    for (const auto& [tpe, texture] : config.curb2_street_texture) {
        auto pmit = config.street_materials.find(tpe);
        if (pmit == config.street_materials.end()) {
            THROW_OR_ABORT("Could not find physics material for type \"" + road_type_to_string(tpe) + '"');
        }
        auto rit = config.street_reflection_map.find(tpe);
        tl_street_curb2.insert(tpe, std::make_shared<TriangleList<CompressedScenePos>>(
            "curb2_" + road_type_to_string(tpe) + name_suffix,
            Material{
                .textures_color = {primary_rendering_resources.get_blend_map_texture(texture)},
                .reflection_map = (rit != config.street_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .occluded_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::LIGHTMAP_BLOBS,
                .occluder_pass = (tpe != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = material_shading((tpe != RoadType::WALL) ? RawShading::CURB : RawShading::DEFAULT, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | pmit->second })); // mixed_texture: terrain_texture
    }
    for (const auto& [tpe, texture] : config.air_curb_street_texture) {
        auto pmit = config.street_materials.find(tpe);
        if (pmit == config.street_materials.end()) {
            THROW_OR_ABORT("Could not find physics material for type \"" + road_type_to_string(tpe) + '"');
        }
        auto rit = config.street_reflection_map.find(tpe);
        tl_air_street_curb.insert(tpe, std::make_shared<TriangleList<CompressedScenePos>>(
            "air_curb_" + road_type_to_string(tpe) + name_suffix,
            Material{
                .textures_color = {primary_rendering_resources.get_blend_map_texture(texture)},
                .reflection_map = (rit != config.street_reflection_map.end())
                    ? rit->second
                    : VariableAndHash<std::string>{},
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                // .wrap_mode_s = curb_wrap_mode_s,
                .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                .shading = material_shading(RawShading::DEFAULT, config),
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL | pmit->second }));
    }
    tl_racing_line = std::make_shared<TriangleList<CompressedScenePos>>(
        "racing_line" + name_suffix,
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .continuous_blending_z_order = 2,
            .depth_func = DepthFunc::EQUAL,
            .blending_pass = BlendingPassType::EARLY,
            .textures_color = {primary_rendering_resources.get_blend_map_texture(config.racing_line_texture)},
            // .wrap_mode_s = WrapMode::CLAMP_TO_EDGE,
            // .wrap_mode_t = WrapMode::REPEAT,
            // depth-func==equal requires aggregation, because the terrain is also aggregated.
            .aggregate_mode = AggregateMode::NODE_TRIANGLES,
            .shading = material_shading(RawShading::DEFAULT, config),
            .draw_distance_noperations = 1000}.compute_color_mode(),
        Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE });
    tl_ditch = std::make_shared<TriangleList<CompressedScenePos>>(
        "ditch" + name_suffix,
        Material{},
        Morphology{ .physics_material = BASE_INVISIBLE_TERRAIN_MATERIAL | PhysicsMaterial::SURFACE_BASE_TARMAC });
    tl_air_support = std::make_shared<TriangleList<CompressedScenePos>>(
        "air_support" + name_suffix,
        Material{
            .textures_color = {primary_rendering_resources.get_blend_map_texture(config.air_support_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::NODE_TRIANGLES,
            .shading = material_shading(RawShading::DEFAULT, config),
            .draw_distance_noperations = 1000}.compute_color_mode(),
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    tl_tunnel_crossing = std::make_shared<TriangleList<CompressedScenePos>>(
        "tunnel_crossing" + name_suffix,
        Material{
            .textures_color = {primary_rendering_resources.get_blend_map_texture(config.tunnel_pipe_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::NODE_TRIANGLES,
            .shading = material_shading(RawShading::DEFAULT, config),
            .draw_distance_noperations = 1000}.compute_color_mode(),
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    tl_tunnel_pipe = std::make_shared<TriangleList<CompressedScenePos>>(
        "tunnel_pipe" + name_suffix,
        Material{
            .textures_color = {primary_rendering_resources.get_blend_map_texture(config.tunnel_pipe_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::NODE_TRIANGLES,
            .shading = material_shading(RawShading::DEFAULT, config),
            .draw_distance_noperations = 1000}.compute_color_mode(),
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    tl_tunnel_bdry = std::make_shared<TriangleList<CompressedScenePos>>(
        "tunnel_bdry" + name_suffix,
        Material{
            .shading = material_shading(RawShading::DEFAULT, config)},
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    tl_entrance[EntranceType::TUNNEL] = std::make_shared<TriangleList<CompressedScenePos>>(
        "tunnel_entrance" + name_suffix,
        Material{
            .shading = material_shading(RawShading::DEFAULT, config)},
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    tl_entrance[EntranceType::BRIDGE] = std::make_shared<TriangleList<CompressedScenePos>>(
        "bridge_entrance" + name_suffix,
        Material{
            .shading = material_shading(RawShading::DEFAULT, config)},
        Morphology{ .physics_material = BASE_VISIBLE_TERRAIN_MATERIAL });
    entrances[EntranceType::TUNNEL];
    entrances[EntranceType::BRIDGE];
    if (config.water.has_value()) {
        std::vector<BlendMapTexture> blend_textures_color;
        blend_textures_color.reserve(config.water->textures_color.size());
        for (const VariableAndHash<std::string>& texture : config.water->textures_color) {
            blend_textures_color.push_back(primary_rendering_resources.get_blend_map_texture(texture));
        }
        std::vector<BlendMapTexture> blend_textures_alpha;
        blend_textures_alpha.reserve(config.water->textures_alpha.size());
        for (const VariableAndHash<std::string>& texture : config.water->textures_alpha) {
            blend_textures_alpha.push_back(primary_rendering_resources.get_blend_map_texture(texture));
        }
        for (const auto& wt : { WaterType::SHALLOW_LAKE, WaterType::UNDEFINED }) {
            tl_water.insert(wt, std::make_shared<TriangleList<CompressedScenePos>>(
                "water" + name_suffix,
                Material{
                    .blend_mode = (config.water->coast.width == (CompressedScenePos)0.f)
                        ? BlendMode::OFF
                        : BlendMode::CONTINUOUS,
                    .blending_pass = (config.water->coast.width == (CompressedScenePos)0.f)
                        ? BlendingPassType::NONE
                        : BlendingPassType::EARLY,
                    .textures_color = blend_textures_color,
                    .textures_alpha = blend_textures_alpha,
                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                    .aggregate_mode = AggregateMode::NODE_TRIANGLES,
                    .shading = material_shading(RawShading::DEFAULT, config),
                    .draw_distance_noperations = 1000}.compute_color_mode(),
                Morphology{ .physics_material = BASE_WATER_MATERIAL }));
        }
    }
}

OsmTriangleLists::~OsmTriangleLists() = default;

#define INSERT(a) a->triangles.insert(a->triangles.end(), other.a->triangles.begin(), other.a->triangles.end());
#define INSERT2(a) for (const auto& [tpe, s] : other.a.map()) a[tpe]->triangles.insert(a[tpe]->triangles.end(), s->triangles.begin(), s->triangles.end());
#define INSERT2p(a) for (const auto& [tpe, s] : other.a->map()) (*a)[tpe]->triangles.insert((*a)[tpe]->triangles.end(), s->triangles.begin(), s->triangles.end());
#define INSERT3(a) for (const auto& s : other.a.list()) a.append(s);
void OsmTriangleLists::insert(const OsmTriangleLists& other) {
    INSERT2p(tl_terrain)
    INSERT2(tl_terrain_visuals)
    INSERT2(tl_terrain_extrusion)
    INSERT2(tl_street_mud_visuals)
    INSERT2(tl_street_crossing)
    INSERT3(tl_street)
    INSERT2(tl_street_curb)
    INSERT2(tl_street_curb2)
    INSERT2(tl_air_street_curb)
    INSERT(tl_air_support)
    INSERT(tl_tunnel_crossing)
    INSERT(tl_tunnel_pipe)
    INSERT(tl_tunnel_bdry)
}
#undef INSERT
#undef INSERT2
#undef INSERT2p
#undef INSERT3

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_street_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{};
    for (const auto& [tpe, e] : tl_street_crossing.map()) {if (tpe != RoadType::WALL) res.push_back(e);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_street_wo_curb_follower() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_racing_line};
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_wall_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{};
    for (const auto& [tpe, e] : tl_street_crossing.map()) {if (tpe == RoadType::WALL) res.push_back(e);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type == RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_street() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support};
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_terrain_nosmooth() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_ditch};
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_terrain_smooth_only_z() const {
    return tls_buildings_ground;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_smooth() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{};
    for (const auto& [_, e] : tl_terrain->map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    // for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    res.insert(res.end(), tls_buildings_ground.begin(), tls_buildings_ground.end());
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_no_backfaces() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_racing_line};
    for (const auto& [_, e] : tl_terrain->map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    // for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_wo_subtraction_and_water() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_racing_line};
    for (const auto& [_, e] : tl_terrain->map()) {if (e->morphology.physics_material != PhysicsMaterial::NONE) res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_wo_subtraction_w_water() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_racing_line};
    for (const auto& [_, e] : tl_water.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain->map()) {if (e->morphology.physics_material != PhysicsMaterial::NONE) res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_raised() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_tunnel_bdry,
        tl_racing_line};
    for (const auto& [_, e] : tl_terrain->map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_smoothed() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_racing_line};
    for (const auto& [_, e] : tl_terrain->map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_extrusion.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    res.insert(res.end(), tls_buildings_ground.begin(), tls_buildings_ground.end());
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_with_vertex_normals() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_tunnel_crossing};
    for (const auto& [_, e] : tl_terrain->map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_terrain_visuals.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_mud_visuals.map()) {res.push_back(e);}
    for (const auto& [tpe, e] : tl_street_crossing.map()) { if (tpe != RoadType::WALL) res.push_back(e);}
    for (const auto& e : tl_street.list()) { if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_no_grass() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_bdry};
    if (tl_terrain->contains(TerrainType::STONE)) res.push_back((*tl_terrain)[TerrainType::STONE]);
    if (tl_terrain->contains(TerrainType::ASPHALT)) res.push_back((*tl_terrain)[TerrainType::ASPHALT]);
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_air_street_curb.map()) {res.push_back(e);}
    return res;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_curb_and_curb2() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{};
    for (const auto& [_, e] : tl_street_curb.map()) {res.push_back(e);}
    for (const auto& [_, e] : tl_street_curb2.map()) {res.push_back(e);}
    return res;
}

bool OsmTriangleLists::has_curb_or_curb2() const {
    for (const auto& [_, e] : tl_street_curb.map()) {if (!e->triangles.empty()) return true;}
    for (const auto& [_, e] : tl_street_curb2.map()) {if (!e->triangles.empty()) return true;}
    return false;
}

std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> OsmTriangleLists::tls_crossing_only() const {
    auto res = std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>{};
    for (const auto& [_, e] : tl_street_crossing.map()) {res.push_back(e);}
    return res;
}

#define INSERT(a) result.insert(result.end(), a->triangles.begin(), a->triangles.end());
#define INSERT2(a) for (const auto& [_, e] : a.map()) { \
    result.insert( \
        result.end(), \
        e->triangles.begin(), \
        e->triangles.end()); \
}
#define INSERT3(a) for (const auto& e : a.list()) { \
    result.insert( \
        result.end(), \
        e.styled_road.triangle_list->triangles.begin(), \
        e.styled_road.triangle_list->triangles.end()); \
}
#define INSERT4(a) for (const auto& e : a) { \
    result.insert( \
        result.end(), \
        e->triangles.begin(), \
        e->triangles.end()); \
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::all_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT2(tl_street_crossing)
    INSERT3(tl_street)
    INSERT2(tl_street_curb)
    INSERT2(tl_street_curb2)
    INSERT(tl_ditch)
    INSERT(tl_entrance.at(EntranceType::TUNNEL))
    INSERT(tl_entrance.at(EntranceType::BRIDGE))
    INSERT4(tls_buildings_ground)
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::entrance_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT(tl_entrance.at(EntranceType::TUNNEL))
    INSERT(tl_entrance.at(EntranceType::BRIDGE))
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::street_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT2(tl_street_crossing)
    INSERT3(tl_street)
    INSERT2(tl_street_curb)
    INSERT2(tl_street_curb2)
    INSERT(tl_ditch)
    INSERT(tl_entrance.at(EntranceType::TUNNEL))
    INSERT(tl_entrance.at(EntranceType::BRIDGE))
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::no_trees_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result = street_hole_triangles();
    if (tl_terrain->contains(TerrainType::FLOWERS)) INSERT((*tl_terrain)[TerrainType::FLOWERS])
    if (tl_terrain->contains(TerrainType::TREES)) INSERT((*tl_terrain)[TerrainType::TREES])
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::building_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT4(tls_buildings_ground)
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::ocean_ground_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT4(tls_ocean_ground)
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::street_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT3(tl_street)
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::street_triangles(RoadType road_type) const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    for (const auto& [p, l] : tl_street.list()) {
        if (p.type == road_type) {
            INSERT(l.triangle_list);
        }
    }
    return result;
}

std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> OsmTriangleLists::ditch_triangles() const {
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> result;
    INSERT(tl_ditch)
    return result;
}

#undef INSERT
#undef INSERT2
#undef INSERT3
#undef INSERT4

namespace Mlib {

template class EntityTypeTriangleList<RoadType>;
template class EntityTypeTriangleList<TerrainType>;
template class EntityTypeTriangleList<WaterType>;

}
