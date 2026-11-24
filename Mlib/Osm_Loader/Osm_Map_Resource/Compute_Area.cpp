#include "Compute_Area.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Elements.hpp>

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
            auto ai = nodes.find(*it);
            auto bi = nodes.find(*s);
            if (ai == nodes.end()) {
                throw std::runtime_error(std::format("Could find node with ID \"{}\"", *it));
            }
            if (bi == nodes.end()) {
                throw std::runtime_error(std::format("Could find node with ID \"{}\"", *s));
            }
            const auto& a = ai->second.position;
            const auto& b = bi->second.position;
            area2 += funpack(b(0) - a(0)) * funpack(b(1) + a(1));
        }
    }
    return area2 / 2. / squared(scale);
}
