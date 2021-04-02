#include "Compute_Area.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

float Mlib::compute_area_clockwise(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    float area2 = 0;
    for (auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            const auto& a = nodes.at(*it).position;
            const auto& b = nodes.at(*s).position;
            area2 += (b(0) - a(0)) * (b(1) + a(1));
        }
    }
    return area2 / 2 / squared(scale);
}
