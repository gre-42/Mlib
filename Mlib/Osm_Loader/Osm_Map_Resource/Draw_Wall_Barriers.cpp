#include "Draw_Wall_Barriers.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Barrier_Style.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Visit_Line_Segments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

void Mlib::draw_wall_barriers(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::map<std::string, BarrierStyle>& barrier_styles)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
            auto smid = std::to_string(mid++);
#pragma GCC diagnostic pop
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
            auto& tl = *tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
                "wall_barriers_" + smid,
                material,
                morphology + (PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE | PhysicsMaterial::ATTR_CONCAVE)));
            if (!bs.cull_faces) {
                tl.morphology += PhysicsMaterial::ATTR_TWO_SIDED;
            }
            tl.material.textures_color = { primary_rendering_resources.get_blend_map_texture(bs.texture) };
            tl.material.blend_mode = bs.blend_mode;
            tl.material.cull_faces = bs.cull_faces;
            tl.material.reorient_uv0 = bs.reorient_uv0;
            tl.material.shading = bs.shading;
            tl.material.compute_color_mode();
            FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
            auto sw = subdivided_way(nodes, bu.way.nd, scale, max_width);
            if (bs.depth == 0.f) {
                float length_mod1 = 0.f;
                visit_line_segments(sw, [&](
                    const FixedArray<CompressedScenePos, 2>& aL,
                    const FixedArray<CompressedScenePos, 2>& aR,
                    const FixedArray<CompressedScenePos, 2>& b,
                    const FixedArray<CompressedScenePos, 2>& c,
                    const FixedArray<CompressedScenePos, 2>& dL,
                    const FixedArray<CompressedScenePos, 2>& dR,
                    SegmentPosition position)
                    {
                        const auto& p0 = b;
                        const auto& p1 = c;
                        float width = (float)std::sqrt(sum(squared(p0 - p1)));
                        float height = (bl.top - bl.bottom) * scale;
                        if (steiner_points != nullptr) {
                            steiner_points->push_back({
                                .position = {b(0), b(1), (CompressedScenePos)0.f},
                                .type = SteinerPointType::WALL });
                            steiner_points->push_back({
                                .position = {c(0), c(1), (CompressedScenePos)0.f},
                                .type = SteinerPointType::WALL });
                        }
                        FixedArray<float, 2> uv = 1.f / scale * uv_scale * bs.uv;
                        // some buildings are clock-wise, others counter-clock-wise
                        ColoredVertex<CompressedScenePos>* pp00a;
                        ColoredVertex<CompressedScenePos>* pp11a;
                        ColoredVertex<CompressedScenePos>* pp01a;
                        ColoredVertex<CompressedScenePos>* pp00b;
                        ColoredVertex<CompressedScenePos>* pp10b;
                        ColoredVertex<CompressedScenePos>* pp11b;
                        tl.draw_rectangle_wo_normals(
                            { p1(0), p1(1), (CompressedScenePos)(bl.bottom * scale) },
                            { p0(0), p0(1), (CompressedScenePos)(bl.bottom * scale) },
                            { p0(0), p0(1), (CompressedScenePos)(bl.top * scale) },
                            { p1(0), p1(1), (CompressedScenePos)(bl.top * scale) },
                            Colors::from_rgb(color),
                            Colors::from_rgb(color),
                            Colors::from_rgb(color),
                            Colors::from_rgb(color),
                            FixedArray<float, 2>{length_mod1, 0.f} * uv,
                            FixedArray<float, 2>{length_mod1 + width, 0.f} * uv,
                            FixedArray<float, 2>{length_mod1 + width, height} * uv,
                            FixedArray<float, 2>{length_mod1, height} * uv,
                            std::nullopt,
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
                        vertex_height_bindings[&pp00a->position] = c;
                        vertex_height_bindings[&pp11a->position] = b;
                        vertex_height_bindings[&pp01a->position] = c;
                        vertex_height_bindings[&pp00b->position] = c;
                        vertex_height_bindings[&pp10b->position] = b;
                        vertex_height_bindings[&pp11b->position] = b;
                        length_mod1 = std::fmod(length_mod1 + width, 1.f / uv(0));
                    });
            } else {
                FixedArray<float, 2> length_mod1{ 0.f };
                visit_line_segments(sw, [&](
                    const FixedArray<CompressedScenePos, 2>& aL,
                    const FixedArray<CompressedScenePos, 2>& aR,
                    const FixedArray<CompressedScenePos, 2>& b,
                    const FixedArray<CompressedScenePos, 2>& c,
                    const FixedArray<CompressedScenePos, 2>& dL,
                    const FixedArray<CompressedScenePos, 2>& dR,
                    SegmentPosition position)
                    {
                        OsmRectangle2D rect = uninitialized;
                        if (!OsmRectangle2D::from_line(
                            rect,
                            aL,
                            aR,
                            b,
                            c,
                            dL,
                            dR,
                            (CompressedScenePos)(scale * bs.depth),
                            (CompressedScenePos)(scale * bs.depth),
                            (CompressedScenePos)(scale * bs.depth),
                            (CompressedScenePos)(scale * bs.depth),
                            (CompressedScenePos)(scale * bs.depth),
                            (CompressedScenePos)(scale * bs.depth)))
                        {
                            lerr() << "Error triangulating barrier " + bu.id;
                        } else {
                            FixedArray<float, 2> width{
                                (float)std::sqrt(sum(squared(rect.p00_ - rect.p10_))),
                                (float)std::sqrt(sum(squared(rect.p01_ - rect.p11_))) };
                            float height = (bl.top - bl.bottom) * scale;
                            if (steiner_points != nullptr) {
                                steiner_points->push_back({
                                    .position = {b(0), b(1), (CompressedScenePos)0.f},
                                    .type = SteinerPointType::WALL });
                                steiner_points->push_back({
                                    .position = {c(0), c(1), (CompressedScenePos)0.f},
                                    .type = SteinerPointType::WALL });
                            }
                            FixedArray<float, 2> uv = 1.f / scale * uv_scale * bs.uv;
                            auto set_height_binding = [&](
                                FixedArray<CompressedScenePos, 3>& addr,
                                const FixedArray<CompressedScenePos, 2>& source){
                                    auto addr2 = FixedArray<CompressedScenePos, 2>{ addr(0), addr(1) };
                                    auto dist = sum(squared(addr2 - source));
                                    auto deviation = std::abs(dist - squared(bs.depth / 2.f));
                                    if (deviation > 1e-2) {
                                        THROW_OR_ABORT("Incorrect vertex height binding. Deviation: " + std::to_string(deviation));
                                    }
                                    vertex_height_bindings[&addr] = source;
                                };
                            // some buildings are clock-wise, others counter-clock-wise
                            {
                                ColoredVertex<CompressedScenePos>* pp00a;
                                ColoredVertex<CompressedScenePos>* pp11a;
                                ColoredVertex<CompressedScenePos>* pp01a;
                                ColoredVertex<CompressedScenePos>* pp00b;
                                ColoredVertex<CompressedScenePos>* pp10b;
                                ColoredVertex<CompressedScenePos>* pp11b;
                                tl.draw_rectangle_wo_normals(
                                    { rect.p10_(0), rect.p10_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p00_(0), rect.p00_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p00_(0), rect.p00_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p10_(0), rect.p10_(1), (CompressedScenePos)(bl.top * scale) },
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    FixedArray<float, 2>{length_mod1(0) - width(0), 0.f} *uv,
                                    FixedArray<float, 2>{length_mod1(0), 0.f} *uv,
                                    FixedArray<float, 2>{length_mod1(0), height} *uv,
                                    FixedArray<float, 2>{length_mod1(0) - width(0), height} *uv,
                                    std::nullopt,
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
                                set_height_binding(pp00a->position, c);
                                set_height_binding(pp00b->position, c);
                                set_height_binding(pp10b->position, b);
                                set_height_binding(pp11b->position, b);
                                set_height_binding(pp11a->position, b);
                                set_height_binding(pp01a->position, c);
                            }
                            {
                                ColoredVertex<CompressedScenePos>* pp00a;
                                ColoredVertex<CompressedScenePos>* pp11a;
                                ColoredVertex<CompressedScenePos>* pp01a;
                                ColoredVertex<CompressedScenePos>* pp00b;
                                ColoredVertex<CompressedScenePos>* pp10b;
                                ColoredVertex<CompressedScenePos>* pp11b;
                                tl.draw_rectangle_wo_normals(
                                    { rect.p01_(0), rect.p01_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p11_(0), rect.p11_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p11_(0), rect.p11_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p01_(0), rect.p01_(1), (CompressedScenePos)(bl.top * scale) },
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    Colors::from_rgb(color),
                                    FixedArray<float, 2>{length_mod1(1), 0.f} *uv,
                                    FixedArray<float, 2>{length_mod1(1) + width(1), 0.f} *uv,
                                    FixedArray<float, 2>{length_mod1(1) + width(1), height} *uv,
                                    FixedArray<float, 2>{length_mod1(1), height} *uv,
                                    std::nullopt,
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
                                set_height_binding(pp00a->position, b);
                                set_height_binding(pp00b->position, b);
                                set_height_binding(pp10b->position, c);
                                set_height_binding(pp11b->position, c);
                                set_height_binding(pp11a->position, c);
                                set_height_binding(pp01a->position, b);
                            }
                            if (any(position & SegmentPosition::START)) {
                                ColoredVertex<CompressedScenePos>* pp00a;
                                ColoredVertex<CompressedScenePos>* pp11a;
                                ColoredVertex<CompressedScenePos>* pp01a;
                                ColoredVertex<CompressedScenePos>* pp00b;
                                ColoredVertex<CompressedScenePos>* pp10b;
                                ColoredVertex<CompressedScenePos>* pp11b;
                                tl.draw_rectangle_wo_normals(
                                    { rect.p00_(0), rect.p00_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p01_(0), rect.p01_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p01_(0), rect.p01_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p00_(0), rect.p00_(1), (CompressedScenePos)(bl.top * scale) },
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    { 0.f, 0.f },
                                    { 1.f, 0.f },
                                    { 1.f, 1.f },
                                    { 0.f, 1.f },
                                    std::nullopt,
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
                                set_height_binding(pp00a->position, b);
                                set_height_binding(pp00b->position, b);
                                set_height_binding(pp10b->position, b);
                                set_height_binding(pp11b->position, b);
                                set_height_binding(pp11a->position, b);
                                set_height_binding(pp01a->position, b);
                            }
                            if (any(position & SegmentPosition::END)) {
                                ColoredVertex<CompressedScenePos>* pp00a;
                                ColoredVertex<CompressedScenePos>* pp11a;
                                ColoredVertex<CompressedScenePos>* pp01a;
                                ColoredVertex<CompressedScenePos>* pp00b;
                                ColoredVertex<CompressedScenePos>* pp10b;
                                ColoredVertex<CompressedScenePos>* pp11b;
                                tl.draw_rectangle_wo_normals(
                                    { rect.p11_(0), rect.p11_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p10_(0), rect.p10_(1), (CompressedScenePos)(bl.bottom * scale) },
                                    { rect.p10_(0), rect.p10_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p11_(0), rect.p11_(1), (CompressedScenePos)(bl.top * scale) },
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    { 0.f, 0.f },
                                    { 1.f, 0.f },
                                    { 1.f, 1.f },
                                    { 0.f, 1.f },
                                    std::nullopt,
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
                                set_height_binding(pp00a->position, c);
                                set_height_binding(pp00b->position, c);
                                set_height_binding(pp10b->position, c);
                                set_height_binding(pp11b->position, c);
                                set_height_binding(pp11a->position, c);
                                set_height_binding(pp01a->position, c);
                            }
                            {
                                ColoredVertex<CompressedScenePos>* pp00a;
                                ColoredVertex<CompressedScenePos>* pp11a;
                                ColoredVertex<CompressedScenePos>* pp01a;
                                ColoredVertex<CompressedScenePos>* pp00b;
                                ColoredVertex<CompressedScenePos>* pp10b;
                                ColoredVertex<CompressedScenePos>* pp11b;
                                tl.draw_rectangle_wo_normals(
                                    { rect.p11_(0), rect.p11_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p10_(0), rect.p10_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p00_(0), rect.p00_(1), (CompressedScenePos)(bl.top * scale) },
                                    { rect.p01_(0), rect.p01_(1), (CompressedScenePos)(bl.top * scale) },
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    Colors::from_rgb(bs.depth_color),
                                    { 0.f, 0.f },
                                    { 1.f, 0.f },
                                    { 1.f, 1.f },
                                    { 0.f, 1.f },
                                    std::nullopt,
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
                                set_height_binding(pp00a->position, c);
                                set_height_binding(pp00b->position, c);
                                set_height_binding(pp10b->position, c);
                                set_height_binding(pp11b->position, b);
                                set_height_binding(pp11a->position, b);
                                set_height_binding(pp01a->position, b);
                            }
                            length_mod1 = {
                                std::fmod(length_mod1(0) - width(0), 1.f / uv(0)),
                                std::fmod(length_mod1(1) + width(1), 1.f / uv(0)) };
                        }
                    });
            }
        }
    }
}
