#include "Bounding_Info.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

BoundingInfo::BoundingInfo(
    const UUVector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::map<std::string, Node>& nodes,
    CompressedScenePos border_width)
    : boundary_min{ fixed_full<CompressedScenePos, 2>(std::numeric_limits<CompressedScenePos>::max()) }
    , boundary_max{ fixed_full<CompressedScenePos, 2>(std::numeric_limits<CompressedScenePos>::lowest()) }
    , border_width{ border_width }
{
    if (bounding_contour.empty()) {
        for (const auto& [_, node] : nodes) {
            boundary_min = minimum(boundary_min, node.position);
            boundary_max = maximum(boundary_max, node.position);
        }
    } else {
        for (const auto& p : bounding_contour) {
            boundary_min = minimum(boundary_min, p);
            boundary_max = maximum(boundary_max, p);
        }
    }
}
