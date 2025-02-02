#include "Draw_Roofs.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Visit_Line_Segments.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

void Mlib::draw_roofs(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const Material& material,
    const Morphology& morphology,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_length)
{
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
        auto& tl = tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
            "roof_" + std::to_string(number),
            material,
            morphology + BASE_VISIBLE_TERRAIN_MATERIAL));
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
        float zz0 = bu.levels.back().top;
        float zz1 = bu.levels.back().top + bu.roof_9_2->height;
        float width = bu.roof_9_2->width;
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
                    (CompressedScenePos)(scale * width),
                    (CompressedScenePos)(scale * width),
                    (CompressedScenePos)(scale * width),
                    (CompressedScenePos)(scale * width),
                    (CompressedScenePos)(scale * width),
                    (CompressedScenePos)(scale * width)))
                {
                    lerr() << "Error triangulating roof " + bu.id;
                } else {
                    rect.p00_ = b;
                    rect.p10_ = c;
                    float uheight = bu.roof_9_2->height;
                    float uwidth = std::sqrt(squared(width) + squared(uheight));
                    tl->draw_rectangle_wo_normals(
                        FixedArray<CompressedScenePos, 3>{rect.p00_(0), rect.p00_(1), max_height + (CompressedScenePos)(zz0 * scale)},
                        FixedArray<CompressedScenePos, 3>{rect.p01_(0), rect.p01_(1), max_height + (CompressedScenePos)(zz1 * scale)},
                        FixedArray<CompressedScenePos, 3>{rect.p11_(0), rect.p11_(1), max_height + (CompressedScenePos)(zz1 * scale)},
                        FixedArray<CompressedScenePos, 3>{rect.p10_(0), rect.p10_(1), max_height + (CompressedScenePos)(zz0 * scale)},
                        Colors::from_rgb(color),
                        Colors::from_rgb(color),
                        Colors::from_rgb(color),
                        Colors::from_rgb(color),
                        { 0.f, 0.f },
                        { uwidth / scale * uv_scale, 0.f },
                        { uwidth / scale * uv_scale, uheight / scale * uv_scale },
                        { 0.f, uheight / scale * uv_scale });
                }
                // draw_node(triangles, nodes.at(aL));
            });
    }
}
