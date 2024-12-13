#include "Draw_Building_Walls.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
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
    std::unordered_map<const FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
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
        std::list<FixedArray<CompressedScenePos, 2, 2>> swG;
        for (const auto& bl : bu.levels) {
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
            auto swGit = swG.begin();
            for (const auto& we : sw) {
                const auto& p0 = we[0];
                const auto& p1 = we[1];
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
                tl->draw_rectangle_wo_normals(
                    {p1(0), p1(1), (CompressedScenePos)(bl.bottom * scale)}, // p00
                    {p0(0), p0(1), (CompressedScenePos)(bl.bottom * scale)}, // p10
                    {p0(0), p0(1), (CompressedScenePos)(bl.top * scale)},    // p11
                    {p1(0), p1(1), (CompressedScenePos)(bl.top * scale)},    // p01
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
                    TriangleTangentErrorBehavior::RAISE,
                    RectangleTriangulationMode::FIRST,
                    &pp00a,
                    &pp11a,
                    &pp01a,
                    &pp00b,
                    &pp10b,
                    &pp11b);
                if (&bl != &*bu.levels.begin()) {
                    if (bl.extra_width != 0.f) {
                        const auto& pG0 = (*swGit)[0];
                        const auto& pG1 = (*swGit)[1];
                        vertex_height_bindings[&pp00a->position] = FixedArray<CompressedScenePos, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp00b->position] = FixedArray<CompressedScenePos, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp10b->position] = FixedArray<CompressedScenePos, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11b->position] = FixedArray<CompressedScenePos, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11a->position] = FixedArray<CompressedScenePos, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp01a->position] = FixedArray<CompressedScenePos, 2>{ pG1(0), pG1(1) };
                    }
                    ++swGit;
                }
            }
            if (&bl == &*bu.levels.begin()) {
                swG = std::move(sw);
            }
        }
    }
}
