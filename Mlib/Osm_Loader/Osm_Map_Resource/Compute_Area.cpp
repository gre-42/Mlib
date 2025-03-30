#include "Compute_Area.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <poly2tri/poly2tri.h>

using namespace Mlib;

double Mlib::compute_area_clockwise(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    double scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double area2 = 0;
    for (auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            const auto& a = nodes.at(*it).position;
            const auto& b = nodes.at(*s).position;
            area2 += funpack(b(0) - a(0)) * funpack(b(1) + a(1));
        }
    }
    return area2 / 2. / squared(scale);
}

double Mlib::compute_area_ccw(
    const std::vector<p2t::Point*>& polygon,
    double scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const p2t::Point& a = **it;
        const p2t::Point& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += (a.x - b.x) * (b.y + a.y);
    }
    return area2 / 2. / squared(scale);
}

double Mlib::compute_area_ccw(
    const std::list<FixedArray<CompressedScenePos, 2>>& polygon,
    double scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const auto& a = *it;
        const auto& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += funpack(a(0) - b(0)) * funpack(b(1) + a(1));
    }
    return area2 / 2. / squared(scale);
}
