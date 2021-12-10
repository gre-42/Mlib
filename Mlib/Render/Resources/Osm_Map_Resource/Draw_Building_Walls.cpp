#include "Draw_Building_Walls.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Facade_Texture.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Vertex_Height_Binding.hpp>

using namespace Mlib;

void Mlib::draw_building_walls(
    std::list<std::shared_ptr<TriangleList>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::map<const FixedArray<float, 3>*, VertexHeightBinding>& vertex_height_bindings,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::vector<std::string>& socle_textures,
    FacadeTextureCycle& ftc)
{
    size_t mid = 0;
    size_t bid = 0;
    for (const auto& bu : buildings) {
        ++bid;
        std::list<FixedArray<FixedArray<float, 2>, 2>> swG;
        for (const auto& bl : bu.levels) {
            tls.push_back(std::make_shared<TriangleList>("building_walls_" + std::to_string(mid++), material));
            std::string texture;
            if (bl.type == BuildingLevelType::SOCLE) {
                if (socle_textures.empty()) {
                    throw std::runtime_error("Socle textures empty");
                }
                texture = socle_textures.at(bid % socle_textures.size()); 
            } else {
                if (ftc.empty()) {
                    throw std::runtime_error("Facade textures empty");
                }
                texture = ftc(bu).name;
            }
            tls.back()->material_.textures = { {.texture_descriptor = {.color = texture}} };
            tls.back()->material_.compute_color_mode();
            FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
            auto sw = smooth_building_level(bu, nodes, max_width, bl.extra_width, bl.extra_width, scale);
            auto swGit = swG.begin();
            for (const auto& we : sw) {
                const auto& p0 = we(0);
                const auto& p1 = we(1);
                float width = std::sqrt(sum(squared(p0 - p1)));
                float height = (bl.top - bl.bottom) * scale;
                if ((steiner_points != nullptr) && (&bl == &*bu.levels.begin())) {
                    steiner_points->push_back({
                        .position = {p0(0), p0(1), 0.f},
                        .type = SteinerPointType::WALL});
                }
                ColoredVertex* pp00a;
                ColoredVertex* pp11a;
                ColoredVertex* pp01a;
                ColoredVertex* pp00b;
                ColoredVertex* pp10b;
                ColoredVertex* pp11b;
                // some buildings are clock-wise, others counter-clock-wise
                tls.back()->draw_rectangle_wo_normals(
                    {p1(0), p1(1), bl.bottom * scale}, // p00
                    {p0(0), p0(1), bl.bottom * scale}, // p10
                    {p0(0), p0(1), bl.top * scale},    // p11
                    {p1(0), p1(1), bl.top * scale},    // p01
                    color,
                    color,
                    color,
                    color,
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
                        vertex_height_bindings[&pp00a->position] = FixedArray<float, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp00b->position] = FixedArray<float, 2>{ pG1(0), pG1(1) };
                        vertex_height_bindings[&pp10b->position] = FixedArray<float, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11b->position] = FixedArray<float, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp11a->position] = FixedArray<float, 2>{ pG0(0), pG0(1) };
                        vertex_height_bindings[&pp01a->position] = FixedArray<float, 2>{ pG1(0), pG1(1) };
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
