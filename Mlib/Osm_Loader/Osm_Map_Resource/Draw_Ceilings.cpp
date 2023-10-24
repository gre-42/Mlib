#include "Draw_Ceilings.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Buildings_Ceiling_Or_Ground.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Material_Colors.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

void Mlib::draw_ceilings(
    std::list<std::shared_ptr<TriangleList<double>>>& tls_buildings,
    const OsmResourceConfig& config,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    draw_buildings_ceiling_or_ground(
        tls_buildings,
        Material{
            .textures_color = { { primary_rendering_resources.get_existing_texture_descriptor(config.ceiling_texture) } },
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
            .aggregate_mode = AggregateMode::ONCE,
            .emissivity = OrderableFixedArray{CEILING_EMISSIVITY * config.emissivity_factor},
            .ambience = OrderableFixedArray{CEILING_AMBIENCE * config.ambience_factor},
            .diffusivity = OrderableFixedArray{CEILING_DIFFUSIVITY * config.diffusivity_factor},
            .specularity = OrderableFixedArray{CEILING_SPECULARITY * config.specularity_factor},
            .draw_distance_noperations = 1000}.compute_color_mode(),
        buildings,
        nodes,
        config.scale,
        config.uv_scale_ceiling,
        1.f,                        // uv_period
        config.max_wall_width,
        DrawBuildingPartType::CEILING);
}
