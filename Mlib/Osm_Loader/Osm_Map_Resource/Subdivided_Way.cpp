#include "Subdivided_Way.hpp"
#include <Mlib/Math/Blend.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

std::list<FixedArray<CompressedScenePos, 2>> Mlib::subdivided_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    double scale,
    double max_length)
{
    std::list<FixedArray<CompressedScenePos, 2>> result;
    for (auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            const auto& p0 = nodes.at(*it).position;
            const auto& p1 = nodes.at(*s).position;
            double width = std::sqrt(sum(squared(p0 - p1)));
            auto refined = linspace_multipliers<double>(std::max<size_t>(2, size_t(width / scale / max_length))).flat_iterable();
            for (auto a = refined.begin(); a != refined.end(); ++a) {
                auto b = a;
                ++b;
                if (b != refined.end() || &*s == &nd.back()) {
                    auto pp0 = blend(p0, p1, a->first, a->second);
                    result.push_back(pp0);
                }
            }
        }
    }
    return result;
}
