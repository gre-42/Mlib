#include "Draw_Building_Walls.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

void Mlib::draw_building_walls(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::map<const FixedArray<double, 3>*, VertexHeightBinding<double>>& vertex_height_bindings,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    float socle_ambient_occlusion,
    const Interp<float, FixedArray<float, 3>>& height_colors)
{
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    size_t mid = 0;
    for (const auto& bu : buildings) {
        std::list<FixedArray<FixedArray<double, 2>, 2>> swG;
        for (const auto& bl : bu.levels) {
            tls.push_back(std::make_shared<TriangleList<double>>(
                "building_walls_" + std::to_string(mid++),
                material,
                PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_CONCAVE | PhysicsMaterial::ATTR_SELF_CONTAINED));
            FixedArray<float, 3> bottom_height_color = height_colors(bl.bottom);
            FixedArray<float, 3> top_height_color = height_colors(bl.top);
            float bottom_ambient_occlusion;
            if (bl.type == BuildingLevelType::SOCLE) {
                bottom_ambient_occlusion = socle_ambient_occlusion;
            } else {
                bottom_ambient_occlusion = 0.f;
            }
            tls.back()->material.textures = { { primary_rendering_resources->get_existing_texture_descriptor(bl.facade_texture_descriptor.name) } };
            tls.back()->material.interior_textures = bl.facade_texture_descriptor.interior_textures;
            tls.back()->material.compute_color_mode();
            FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
            auto sw = smooth_building_level(bu, nodes, max_width, bl.extra_width, bl.extra_width, scale);
            auto swGit = swG.begin();
            for (const auto& we : sw) {
                const auto& p0 = we(0);
                const auto& p1 = we(1);
                float width = (float)std::sqrt(sum(squared(p0 - p1)));
                float height = (bl.top - bl.bottom) * scale;
                if ((steiner_points != nullptr) && (&bl == &*bu.levels.begin())) {
                    steiner_points->push_back({
                        .position = {p0(0), p0(1), 0.f},
                        .type = SteinerPointType::WALL});
                }
                ColoredVertex<double>* pp00a;
                ColoredVertex<double>* pp11a;
                ColoredVertex<double>* pp01a;
                ColoredVertex<double>* pp00b;
                ColoredVertex<double>* pp10b;
                ColoredVertex<double>* pp11b;
                // some buildings are clock-wise, others counter-clock-wise
                tls.back()->draw_rectangle_wo_normals(
                    {p1(0), p1(1), bl.bottom * scale}, // p00
                    {p0(0), p0(1), bl.bottom * scale}, // p10
                    {p0(0), p0(1), bl.top * scale},    // p11
                    {p1(0), p1(1), bl.top * scale},    // p01
                    color * (1.f - bottom_ambient_occlusion) * bottom_height_color,
                    color * (1.f - bottom_ambient_occlusion) * bottom_height_color,
                    color * top_height_color,
                    color * top_height_color,
                    {0.f, 0.f},
                    {width / scale * uv_scale, 0.f},
                    {width / scale * uv_scale, height / scale * uv_scale},
                    {0.f, height / scale * uv_scale},
                    {},
                    {},
                    {},
                    {},
                    TriangleNormalErrorBehavior::RAISE,
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
                        const auto& pG0 = (*swGit)(0);
                        const auto& pG1 = (*swGit)(1);
                        vertex_height_bindings[&pp00a->position] = FixedArray<double, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp00b->position] = FixedArray<double, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp10b->position] = FixedArray<double, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11b->position] = FixedArray<double, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11a->position] = FixedArray<double, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp01a->position] = FixedArray<double, 2>{ pG1(0), pG1(1) };
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
