#include "Draw_Building_Walls.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Iterator/Reverse_Iterator.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Material_Colors.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Render/Renderables/Color_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>

using namespace Mlib;

void Mlib::draw_building_walls(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    float snap_length_ratio,
    float snap_length_angle,
    float socle_ambient_occlusion,
    const UUInterp<float, FixedArray<float, 3>>& height_colors,
    ColorCycle& color_cycle)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    size_t mid = 0;
    for (const auto& bu : buildings) {
        auto outline = smooth_building_level_outline(
            bu,
            nodes,
            scale,
            max_width,
            DrawBuildingPartType::GROUND,
            BuildingDetailType::COMBINED);
        auto max_height = std::numeric_limits<CompressedScenePos>::lowest();
        for (const auto& v : outline.outline) {
            auto it = displacements.find(OrderableFixedArray{v.orig});
            if (it == displacements.end()) {
                lwarn() << "Displacements not found for building " + bu.id;
                max_height = std::numeric_limits<CompressedScenePos>::lowest();
                break;
            }
            max_height = std::max(max_height, it->second(2));
        }
        if (max_height == std::numeric_limits<CompressedScenePos>::lowest()) {
            continue;
        }
        for (const auto& [i, bl] : enumerate(bu.levels)) {
            const auto& tl = tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
                "building_walls_" + std::to_string(mid++),
                material,
                morphology + bl.facade_texture_descriptor.material + BASE_VISIBLE_TERRAIN_MATERIAL));
            tl->material.shading = material_shading(bl.facade_texture_descriptor.material);
            FixedArray<float, 3> bottom_height_color = height_colors(bl.bottom);
            FixedArray<float, 3> top_height_color = height_colors(bl.top);
            float bottom_ambient_occlusion;
            if (bl.type == BuildingLevelType::SOCLE) {
                bottom_ambient_occlusion = socle_ambient_occlusion;
            } else {
                bottom_ambient_occlusion = 0.f;
            }
            tl->material.textures_color.reserve(bl.facade_texture_descriptor.names.size());
            for (const auto& name : bl.facade_texture_descriptor.names) {
                tl->material.textures_color.push_back(primary_rendering_resources.get_blend_map_texture(name));
            }
            tl->material.interior_textures = bl.facade_texture_descriptor.interior_textures;
            tl->material.compute_color_mode();
            FixedArray<float, 3> color = color_cycle.empty() || bu.way.tags.contains("color")
                ? parse_color(bu.way.tags, "color", building_color)
                : color_cycle.try_multiple_times(100, BuildingInformation{}).color;
            auto get_uv_ratio = [&bl, &scale](const std::list<BuildingSegment>& sw){
                double w = 0.f;
                for (const auto& we : sw) {
                    float width = (float)std::sqrt(sum(squared(we.indented[0] - we.indented[1])));
                    w += width / scale;
                }
                float e = bl.facade_texture_descriptor.interior_textures.facade_edge_size(0);
                float i = bl.facade_texture_descriptor.interior_textures.facade_inner_size(0);
                float x = bl.facade_texture_descriptor.interior_textures.interior_size(0);
                int n = float_to_integral<int>(std::round((w + i - 2 * e) / (x + i)));
                if (n <= 0) {
                    return 0.f;
                }
                double w2 = 2 * e + ((float)n - 1) * i + (float)n * x;
                return (float)(w2 / w);
            };
            auto swg = smooth_building_level(bu, nodes, max_width, bl.extra_width, bl.extra_width, scale);
            auto swl = straight_building_level(swg, snap_length_angle);
            if (swl.empty()) {
                THROW_OR_ABORT("Building straight segment list empty");
            }
            for (const auto& sw : swl) {
                if (sw.empty()) {
                    THROW_OR_ABORT("Building straight segment empty");
                }
                FixedArray<float, 2> uv = 1.f / scale * uv_scale * fixed_ones<float, 2>();
                std::optional<float> interiormap_uscale;
                if (!tl->material.interior_textures.empty()) {
                    interiormap_uscale = get_uv_ratio(sw);
                    if (!std::isfinite(*interiormap_uscale)) {
                        THROW_OR_ABORT("Building wall length ratio not finite");
                    }
                    // uv(0) *= *interiormap_uscale;
                }
                float length_mod1_uv = 0.f;
                float length_pos = 0.f;
                if (interiormap_uscale.has_value()) {
                    length_pos = bl.facade_texture_descriptor.interior_textures.facade_edge_size(0);
                }
                for (const auto& we : reverse(sw)) {
                    const auto& p0 = displacements.at(OrderableFixedArray{we.indented[0]});
                    const auto& p1 = displacements.at(OrderableFixedArray{we.indented[1]});
                    float width = (float)std::sqrt(sum(squared(we.indented[0] - we.indented[1])));
                    float height = (bl.top - bl.bottom) * scale;
                    if ((steiner_points != nullptr) && (&bl == &*bu.levels.begin())) {
                        steiner_points->push_back({
                            .position = {p0(0), p0(1), (CompressedScenePos)0.f},
                            .type = SteinerPointType::WALL});
                    }
                    ColoredVertex<CompressedScenePos>* pp00a;
                    ColoredVertex<CompressedScenePos>* pp11a;
                    ColoredVertex<CompressedScenePos>* pp01a;
                    ColoredVertex<CompressedScenePos>* pp00b;
                    ColoredVertex<CompressedScenePos>* pp10b;
                    ColoredVertex<CompressedScenePos>* pp11b;
                    // some buildings are clock-wise, others counter-clock-wise
                    auto h0 = (i == 0) ? p0(2) : max_height;
                    auto h1 = (i == 0) ? p1(2) : max_height;
                    tl->draw_rectangle_wo_normals(
                        {p1(0), p1(1), h1         + (CompressedScenePos)(bl.bottom * scale)}, // p00
                        {p0(0), p0(1), h0         + (CompressedScenePos)(bl.bottom * scale)}, // p10
                        {p0(0), p0(1), max_height + (CompressedScenePos)(bl.top * scale)},    // p11
                        {p1(0), p1(1), max_height + (CompressedScenePos)(bl.top * scale)},    // p01
                        Colors::from_rgb(color * (1.f - bottom_ambient_occlusion) * bottom_height_color),
                        Colors::from_rgb(color * (1.f - bottom_ambient_occlusion) * bottom_height_color),
                        Colors::from_rgb(color * top_height_color),
                        Colors::from_rgb(color * top_height_color),
                        FixedArray<float, 2>{length_mod1_uv, 0.f} * uv,
                        FixedArray<float, 2>{length_mod1_uv + width, 0.f} * uv,
                        FixedArray<float, 2>{length_mod1_uv + width, height} * uv,
                        FixedArray<float, 2>{length_mod1_uv, height} * uv,
                        interiormap_uscale.has_value()
                            ? FixedArray<float, 4>{
                                -length_pos / scale,
                                *interiormap_uscale,
                                -bl.facade_texture_descriptor.interior_textures.facade_edge_size(1),
                                1.f}
                            : std::optional<FixedArray<float, 4>>(),
                        {},
                        {},
                        {},
                        {},
                        NormalVectorErrorBehavior::THROW,
                        TriangleTangentErrorBehavior::THROW,
                        RectangleTriangulationMode::FIRST,
                        DelaunayErrorBehavior::THROW,
                        &pp00a,
                        &pp11a,
                        &pp01a,
                        &pp00b,
                        &pp10b,
                        &pp11b);
                    length_mod1_uv = std::fmod(length_mod1_uv + width, scale / uv_scale);
                    length_pos -= width * interiormap_uscale.value_or(1.f);
                }
            }
        }
    }
}
