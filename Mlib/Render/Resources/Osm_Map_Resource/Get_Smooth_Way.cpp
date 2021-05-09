#include "Get_Smooth_Way.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

std::list<FixedArray<float, 2>> Mlib::smooth_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    float scale,
    float max_length)
{
    std::list<FixedArray<float, 2>> result;
    for (auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            auto p0 = nodes.at(*it).position;
            auto p1 = nodes.at(*s).position;
            float width = std::sqrt(sum(squared(p0 - p1)));
            auto refined = linspace_multipliers<float>(std::max(2, int(width / scale / max_length))).flat_iterable();
            for (auto a = refined.begin(); a != refined.end(); ++a) {
                auto b = a;
                ++b;
                if (b != refined.end() || &*s == &nd.back()) {
                    auto pp0 = a->first * p0 + a->second * p1;
                    result.push_back(pp0);
                }
            }
        }
    }
    return result;
}
