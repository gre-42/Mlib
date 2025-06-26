#include "Draw_Ceilings.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material_Configuration/Material_Colors.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Buildings_Ceiling_Or_Ground.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::draw_ceilings(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_buildings,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const OsmResourceConfig& config,
    const std::list<Building>& buildings,
    const Morphology& morphology,
    const std::map<std::string, Node>& nodes,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    ContourDetectionStrategy contour_detection_strategy)
{
    if (config.ceiling_texture->empty()) {
        THROW_OR_ABORT("Ceiling texture empty");
    }
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    draw_buildings_ceiling_or_ground(
        tls_buildings,
        &displacements,
        Material{
            .textures_color = { { primary_rendering_resources.get_texture_descriptor(config.ceiling_texture) } },
            .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
            .aggregate_mode = (config.building_cluster_width == 0)
                ? AggregateMode::SORTED_CONTINUOUSLY
                : AggregateMode::NODE_OBJECT,
            .shading = material_shading(RawShading::CEILING, config),
            .draw_distance_noperations = 1000}.compute_color_mode(),
        morphology,
        buildings,
        nodes,
        config.scale,
        config.triangulation_scale,
        config.uv_scale_ceiling,
        1.f,                        // uv_period
        config.max_wall_width,
        DrawBuildingPartType::CEILING,
        contour_triangles_filename,
        contour_filename,
        triangle_filename,
        contour_detection_strategy);
}
