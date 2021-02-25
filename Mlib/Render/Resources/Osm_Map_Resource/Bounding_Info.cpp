#include "Bounding_Info.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

BoundingInfo::BoundingInfo(
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::map<std::string, Node>& nodes,
    float border_width)
: border_width{border_width}
{
    boundary_min = fixed_full<float, 2>(INFINITY);
    boundary_max = fixed_full<float, 2>(-INFINITY);
    if (bounding_contour.empty()) {
        for (const auto& n : nodes) {
            boundary_min = minimum(boundary_min, n.second.position);
            boundary_max = maximum(boundary_max, n.second.position);
        }
    } else {
        for (const auto& p : bounding_contour) {
            boundary_min = minimum(boundary_min, p);
            boundary_max = maximum(boundary_max, p);
        }
    }
}
