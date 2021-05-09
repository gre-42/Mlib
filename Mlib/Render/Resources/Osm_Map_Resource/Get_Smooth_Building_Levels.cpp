#include "Get_Smooth_Building_Levels.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Smooth_Way.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Rectangle.hpp>

using namespace Mlib;

std::list<Rectangle> Mlib::smooth_building_level(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    float max_length,
    float width0,
    float width1,
    float scale)
{
    if (bu.way.nd.empty()) {
        throw std::runtime_error("Building " + bu.id + ": outline is empty");
    }
    if (bu.way.nd.front() != bu.way.nd.back()) {
        throw std::runtime_error("Cannot compute smooth level of building " + bu.id + ": outline not closed");
    }
    std::list<Rectangle> result;
    auto sw = smooth_way(
        nodes,
        bu.way.nd,
        scale,
        max_length);
    sw.erase(sw.begin());
    if (bu.area < 0) {
        sw.reverse();
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
        Rectangle rect;
        if (!Rectangle::from_line(
                rect,
                *a,
                *a,
                *b,
                *c,
                *d,
                *d,
                scale * width0,
                scale * width1,
                scale * width0,
                scale * width1,
                scale * width0,
                scale * width1))
        {
            throw std::runtime_error("Error triangulating level of building " + bu.id);
        } else {
            result.push_back(rect);
        }
        // draw_node(triangles, nodes.at(*a));
        ++a;
        if (a == sw.end()) {
            a = sw.begin();
        }
    }
    return result;
}

std::list<FixedArray<float, 2>> Mlib::smooth_building_level_outline(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    float scale,
    float max_length,
    DrawBuildingPartType tpe)
{
    const BuildingLevel& bl = (tpe == DrawBuildingPartType::CEILING)
        ? bu.levels.back()
        : bu.levels.front();
    auto sw = smooth_building_level(
        bu,
        nodes,
        max_length,
        bl.extra_width,
        bl.extra_width,
        scale);
    std::list<FixedArray<float, 2>> result;
    for (const auto& w : sw) {
        result.push_back(w.p00_);
    }
    return result;
}
