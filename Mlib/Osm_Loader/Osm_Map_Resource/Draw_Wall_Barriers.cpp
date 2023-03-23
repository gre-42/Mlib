#include "Draw_Wall_Barriers.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Barrier_Style.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

void Mlib::draw_wall_barriers(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::map<std::string, BarrierStyle>& barrier_styles)
{
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    std::vector<BarrierStyle> barrier_styles_vector;
    barrier_styles_vector.reserve(barrier_styles.size());
    for (const auto& v : barrier_styles) {
        barrier_styles_vector.push_back(v.second);
    }
    size_t mid = 0;
    size_t bid = 0;
    for (const auto& bu : buildings) {
        ++bid;
        for (const auto& bl : bu.levels) {
            tls.push_back(std::make_shared<TriangleList<double>>(
                "wall_barriers_" + std::to_string(mid++),
                material,
                PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_TWO_SIDED | PhysicsMaterial::ATTR_CONCAVE));
            auto get_style = [&]() -> const BarrierStyle& {
                if (bu.style.empty()) {
                    if (barrier_styles.empty()) {
                        THROW_OR_ABORT("Barrier textures empty");
                    }
                    return barrier_styles_vector.at(bid % barrier_styles.size());
                } else {
                    if (barrier_styles.find(bu.style) == barrier_styles.end()) {
                        THROW_OR_ABORT("Could not find barrier style with name \"" + bu.style + '"');
                    }
                    return barrier_styles.at(bu.style);
                }
            };
            const BarrierStyle& bs = get_style();
            tls.back()->material_.textures = { primary_rendering_resources->get_blend_map_texture(bs.texture) };
            tls.back()->material_.blend_mode = bs.blend_mode;
            tls.back()->material_.wrap_mode_t = bs.wrap_mode_t;
            tls.back()->material_.reorient_uv0 = bs.reorient_uv0;
            tls.back()->material_.ambience *= bs.ambience;
            tls.back()->material_.diffusivity *= bs.diffusivity;
            tls.back()->material_.specularity *= bs.specularity;
            tls.back()->material_.compute_color_mode();
            FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
            auto sw = subdivided_way(nodes, bu.way.nd, scale, max_width);
            float length_mod1 = 0.f;
            for (auto it = sw.begin(); it != sw.end(); ++it) {
                auto s = it;
                ++s;
                if (s != sw.end()) {
                    const auto& p0 = *s;
                    const auto& p1 = *it;
                    float width = (float)std::sqrt(sum(squared(p0 - p1)));
                    float height = (bl.top - bl.bottom) * scale;
                    if (steiner_points != nullptr) {
                        steiner_points->push_back({
                            .position = {p0(0), p0(1), 0.f},
                            .type = SteinerPointType::WALL});
                    }
                    FixedArray<float, 2> uv = 1.f / scale * uv_scale * bs.uv;
                    // some buildings are clock-wise, others counter-clock-wise
                    tls.back()->draw_rectangle_wo_normals(
                        {p1(0), p1(1), bl.bottom * scale},
                        {p0(0), p0(1), bl.bottom * scale},
                        {p0(0), p0(1), bl.top * scale},
                        {p1(0), p1(1), bl.top * scale},
                        color,
                        color,
                        color,
                        color,
                        FixedArray<float, 2>{length_mod1, 0.f} * uv,
                        FixedArray<float, 2>{length_mod1 + width, 0.f} * uv,
                        FixedArray<float, 2>{length_mod1 + width, height} * uv,
                        FixedArray<float, 2>{length_mod1, height} * uv);
                    length_mod1 = std::fmod(length_mod1 + width, 1.f / uv(0));
                }
            }
        }
    }
}
