#include "Draw_Roofs.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle.hpp>
#include <iostream>

using namespace Mlib;

void Mlib::draw_roofs(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    const Material& material,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float width,
    float scale,
    float z0,
    float z1)
{
    for (const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            throw std::runtime_error("Cannot draw roof of building " + bu.id + ": outline not closed");
        }
        tls.push_back(std::make_shared<TriangleList<double>>(
            "roofs",
            material,
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
        auto way1 = bu.way.nd;
        way1.erase(way1.begin());
        float zz0 = z0;
        float zz1 = z1;
        if (bu.area < 0) {
            std::swap(zz0, zz1);
        }
        auto a = way1.begin();
        for (size_t i = 0; i < way1.size(); ++i) {
            auto b = a;
            ++b;
            if (b == way1.end()) {
                b = way1.begin();
            }
            auto c = b;
            ++c;
            if (c == way1.end()) {
                c = way1.begin();
            }
            auto d = c;
            ++d;
            if (d == way1.end()) {
                d = way1.begin();
            }
            Rectangle rect;
            if (!Rectangle::from_line(
                    rect,
                    nodes.at(*a).position,
                    nodes.at(*a).position,
                    nodes.at(*b).position,
                    nodes.at(*c).position,
                    nodes.at(*d).position,
                    nodes.at(*d).position,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width))
            {
                std::cerr << "Error triangulating roof " + bu.id << std::endl;
            } else {
                rect.draw_z(*tls.back(), zz0 * scale, zz1 * scale, color);
            }
            // draw_node(triangles, nodes.at(*a));
            ++a;
            if (a == way1.end()) {
                a = way1.begin();
            }
        }
    }
}
