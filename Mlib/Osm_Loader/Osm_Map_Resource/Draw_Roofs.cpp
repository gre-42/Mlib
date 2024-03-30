#include "Draw_Roofs.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Base_Materials.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

void Mlib::draw_roofs(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    const Material& material,
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
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            THROW_OR_ABORT("Cannot draw roof of building " + bu.id + ": outline not closed");
        }
        tls.push_back(std::make_shared<TriangleList<double>>(
            "roof_" + std::to_string(number),
            material,
            BASE_VISIBLE_TERRAIN_MATERIAL));
        auto sw = subdivided_way(
            nodes,
            bu.way.nd,
            scale,
            max_length);
        sw.erase(sw.begin());
        float zz0 = bu.levels.back().top;
        float zz1 = bu.levels.back().top + bu.roof_9_2.value().height;
        float width = bu.roof_9_2.value().width;
        if (bu.area < 0) {
            std::swap(zz0, zz1);
        }
        auto a = sw.begin();
        for (size_t i = 0; i < sw.size(); ++i) {
            auto b = a;
            ++b;
            if (b == sw.end()) {
                b = sw.begin();
            }
            auto c = b;
            ++c;
            if (c == sw.end()) {
                c = sw.begin();
            }
            auto d = c;
            ++d;
            if (d == sw.end()) {
                d = sw.begin();
            }
            OsmRectangle2D rect;
            if (!OsmRectangle2D::from_line(
                    rect,
                    *a,
                    *a,
                    *b,
                    *c,
                    *d,
                    *d,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width))
            {
                std::cerr << "Error triangulating roof " + bu.id << std::endl;
            } else {
                if (bu.area < 0) {
                    rect.p01_ = *b;
                    rect.p11_ = *c;
                } else {
                    rect.p00_ = *b;
                    rect.p10_ = *c;
                }
                float uheight = bu.roof_9_2.value().height;
                float uwidth = std::sqrt(squared(width) + squared(uheight));
                rect.draw_z(
                    *tls.back(),
                    zz0 * scale,
                    zz1 * scale,
                    color,
                    color,
                    color,
                    color,
                    {0.f, 0.f},
                    {uwidth / scale * uv_scale, 0.f},
                    {uwidth / scale * uv_scale, uheight / scale * uv_scale},
                    {0.f, uheight / scale * uv_scale});
            }
            // draw_node(triangles, nodes.at(*a));
            ++a;
            if (a == sw.end()) {
                a = sw.begin();
            }
        }
    }
}
