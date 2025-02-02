#include "Draw_Building_Walls.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
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
    float socle_ambient_occlusion,
    const UUInterp<float, FixedArray<float, 3>>& height_colors)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    size_t mid = 0;
    for (const auto& bu : buildings) {
        auto outline = smooth_building_level_outline(bu, nodes, scale, max_width, DrawBuildingPartType::GROUND);
        auto max_height = std::numeric_limits<CompressedScenePos>::lowest();
        for (const auto& v : outline.outline) {
            auto it = displacements.find(OrderableFixedArray{v});
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
                morphology + BASE_VISIBLE_TERRAIN_MATERIAL));
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
            FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
            auto sw = smooth_building_level(bu, nodes, max_width, bl.extra_width, bl.extra_width, scale);
            for (const auto& we : sw) {
                const auto& p0 = displacements.at(OrderableFixedArray{we[0]});
                const auto& p1 = displacements.at(OrderableFixedArray{we[1]});
                float width = (float)std::sqrt(sum(squared(p0 - p1)));
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
                    {0.f, 0.f},
                    {width / scale * uv_scale, 0.f},
                    {width / scale * uv_scale, height / scale * uv_scale},
                    {0.f, height / scale * uv_scale},
                    {},
                    {},
                    {},
                    {},
                    NormalVectorErrorBehavior::THROW,
                    TriangleTangentErrorBehavior::THROW,
                    RectangleTriangulationMode::FIRST,
                    &pp00a,
                    &pp11a,
                    &pp01a,
                    &pp00b,
                    &pp10b,
                    &pp11b);
            }
        }
    }
}
