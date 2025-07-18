#include "Draw_Roofs.hpp"
#include <Mlib/Geometry/Material_Configuration/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Morphology.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way_Vertex.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Visit_Line_Segments.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

void Mlib::draw_roofs(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const SceneNodeResources& scene_node_resources,
    const VariableAndHash<std::string>& model_name,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const Material& roof_material,
    const Material& rail_material,
    const GetMorphology& get_morphology,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_length)
{
    std::list<std::shared_ptr<ColoredVertexArray<float>>> model_triangles;
    if (!model_name->empty()) {
        model_triangles = scene_node_resources.get_single_precision_arrays(model_name, ColoredVertexArrayFilter{});
    }
    for (auto&& [number, bu] : enumerate(buildings)) {
        if (!bu.roof_9_2.has_value()) {
            continue;
        }
        if (bu.way.nd.empty()) {
            lerr() << "Building " + bu.id + ": outline is empty";
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            THROW_OR_ABORT("Cannot draw roof of building " + bu.id + ": outline not closed");
        }
        auto draw = [&](BuildingDetailType tpe){
            std::shared_ptr<TriangleList<CompressedScenePos>> tl_roof_var = nullptr;
            auto get_tl_roof = [&](){
                if (tl_roof_var == nullptr) {
                    tl_roof_var = tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
                        "roof_" + std::to_string(number),
                        roof_material,
                        get_morphology[tpe] + BASE_VISIBLE_TERRAIN_MATERIAL));
                }
                return tl_roof_var;
            };
            std::shared_ptr<TriangleList<CompressedScenePos>> tl_rail_var = nullptr;
            auto get_tl_rail = [&](){
                if (tl_rail_var == nullptr) {
                    tl_rail_var = tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
                        "roof_rail_" + std::to_string(number),
                        rail_material,
                        get_morphology[BuildingDetailType::HIGH] + BASE_VISIBLE_TERRAIN_MATERIAL));
                }
                return tl_rail_var;
            };
            auto nd = bu.way.nd;
            if (bu.area < 0) {
                nd.reverse();
            }
            auto sw = subdivided_way(
                nodes,
                nd,
                scale,
                max_length);
            if (bu.levels.empty()) {
                THROW_OR_ABORT("Building has no levels");
            }
            auto max_height = std::numeric_limits<CompressedScenePos>::lowest();
            for (const auto& v : sw) {
                auto it = displacements.find(make_orderable(v.position()));
                if (it == displacements.end()) {
                    lwarn() << "Displacements not found for building " + bu.id;
                    max_height = std::numeric_limits<CompressedScenePos>::lowest();
                    break;
                }
                max_height = std::max(max_height, it->second(2));
            }
            if (max_height == std::numeric_limits<CompressedScenePos>::lowest()) {
                return;
            }
            float zz0 = bu.levels.back().top;
            float zz1 = bu.levels.back().top + bu.roof_9_2->height;
            float width = bu.roof_9_2->width;
            float uheight = bu.roof_9_2->height;
            float uwidth = std::sqrt(squared(width) + squared(uheight));
            float length_mod1 = 0.f;
            visit_line_segments(sw, [&](
                const SubdividedWayVertex& aL,
                const SubdividedWayVertex& aR,
                const SubdividedWayVertex& b,
                const SubdividedWayVertex& c,
                const SubdividedWayVertex& dL,
                const SubdividedWayVertex& dR,
                SegmentPosition position)
                {
                    OsmRectangle2D rect = uninitialized;
                    if (!OsmRectangle2D::from_line(
                        rect,
                        aL.position(),
                        aR.position(),
                        b.position(),
                        c.position(),
                        dL.position(),
                        dR.position(),
                        (CompressedScenePos)(scale * width),
                        (CompressedScenePos)(scale * width),
                        (CompressedScenePos)(scale * width),
                        (CompressedScenePos)(scale * width),
                        (CompressedScenePos)(scale * width),
                        (CompressedScenePos)(scale * width)))
                    {
                        lerr() << "Error triangulating roof " + bu.id;
                    } else {
                        float width = (float)std::sqrt(sum(squared(b.position() - c.position())));
                        if (model_triangles.empty() ||
                            (tpe == BuildingDetailType::LOW) ||
                            (tpe == BuildingDetailType::COMBINED))
                        {
                            rect.p00_ = b.position();
                            rect.p10_ = c.position();
                            FixedArray<CompressedScenePos, 2, 2, 3> rect3{
                                FixedArray<CompressedScenePos, 2, 3>{
                                    FixedArray<CompressedScenePos, 3>{rect.p00_(0), rect.p00_(1), max_height + (CompressedScenePos)(zz0 * scale)},
                                    FixedArray<CompressedScenePos, 3>{rect.p01_(0), rect.p01_(1), max_height + (CompressedScenePos)(zz1 * scale)}},
                                FixedArray<CompressedScenePos, 2, 3>{
                                    FixedArray<CompressedScenePos, 3>{rect.p10_(0), rect.p10_(1), max_height + (CompressedScenePos)(zz0 * scale)},
                                    FixedArray<CompressedScenePos, 3>{rect.p11_(0), rect.p11_(1), max_height + (CompressedScenePos)(zz1 * scale)}}};
                            get_tl_roof()->draw_rectangle_wo_normals(
                                rect3[0][0],
                                rect3[0][1],
                                rect3[1][1],
                                rect3[1][0],
                                Colors::from_rgb(color),
                                Colors::from_rgb(color),
                                Colors::from_rgb(color),
                                Colors::from_rgb(color),
                                { 0.f, 0.f },
                                { uwidth / scale * uv_scale, 0.f },
                                { uwidth / scale * uv_scale, uheight / scale * uv_scale },
                                { 0.f, uheight / scale * uv_scale },
                                std::nullopt,
                                {},
                                {},
                                {},
                                {},
                                NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                                TriangleTangentErrorBehavior::WARN);
                        } else if (tpe == BuildingDetailType::HIGH) {
                            WarpedSegment2D ws{rect};
                            for (const auto& cva : model_triangles) {
                                for (const auto& t : cva->triangles) {
                                    FixedArray<CompressedScenePos, 3, 3> tt = uninitialized;
                                    for (size_t i = 0; i < 3; ++i) {
                                        if (all(t(i).position == FixedArray<float, 3>{0.f, -1.f, 1.f})) {
                                            // Calculate left roof top vertex
                                            tt[i] = FixedArray<CompressedScenePos, 3>{
                                                rect.p01_(0),
                                                rect.p01_(1),
                                                max_height + (CompressedScenePos)(zz1 * scale)};
                                        } else if (all(t(i).position == FixedArray<float, 3>{0.f, 1.f, 1.f})) {
                                            // Calculate right roof top vertex
                                            tt[i] = FixedArray<CompressedScenePos, 3>{
                                                rect.p11_(0),
                                                rect.p11_(1),
                                                max_height + (CompressedScenePos)(zz1 * scale)};
                                        } else {
                                            // Calculate regular vertex (rail or not roof top)
                                            tt[i] = ws.warp(t(i).position.casted<double>(), scale, 1, (CompressedScenePos)uheight);
                                            tt(i, 2) += max_height + CompressedScenePos(zz0 * scale);
                                        }
                                    }
                                    if (cva->name.name() == "roof") {
                                        auto sc = FixedArray<float, 2>{
                                            width / scale * uv_scale,
                                            uwidth / scale * uv_scale
                                        };
                                        get_tl_roof()->draw_triangle_wo_normals(
                                            tt[0],
                                            tt[1],
                                            tt[2],
                                            t(0).color,
                                            t(1).color,
                                            t(2).color,
                                            t(0).uv * sc,
                                            t(1).uv * sc,
                                            t(2).uv * sc,
                                            std::nullopt,
                                            {},
                                            {},
                                            {},
                                            NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                                            TriangleTangentErrorBehavior::WARN);
                                    } else if (cva->name.name() == "rail") {
                                        get_tl_rail()->draw_triangle_wo_normals(
                                            tt[0],
                                            tt[1],
                                            tt[2],
                                            t(0).color,
                                            t(1).color,
                                            t(2).color,
                                            t(0).uv,
                                            t(1).uv,
                                            t(2).uv,
                                            std::nullopt,
                                            {},
                                            {},
                                            {},
                                            NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                                            TriangleTangentErrorBehavior::WARN);
                                    } else {
                                        THROW_OR_ABORT("Unknown mesh name: \"" + cva->name.full_name() + '"');
                                    }
                                }
                            }
                        }
                    }
                    // draw_node(triangles, nodes.at(aL));
                    length_mod1 = std::fmod(length_mod1 + width, scale / uv_scale);
                });
        };
        try {
            if (model_triangles.empty()) {
                draw(BuildingDetailType::COMBINED);
            } else {
                draw(BuildingDetailType::HIGH);
                draw(BuildingDetailType::LOW);
            }
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error triangulating roof \"" + bu.id + "\": " + e.what());
        }
    }
}
