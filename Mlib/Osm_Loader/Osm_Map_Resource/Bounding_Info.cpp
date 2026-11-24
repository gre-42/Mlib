#include "Bounding_Info.hpp"
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

BoundingInfo::BoundingInfo(
    const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::map<std::string, Node>& nodes,
    CompressedScenePos border_width_explicit,
    CompressedScenePos border_width_auto,
    CompressedScenePos segment_length)
    : aabb{ AxisAlignedBoundingBox<CompressedScenePos, 2>::empty() }
    , segment_length{ segment_length }
{
    if (bounding_contour.empty()) {
        border_width = border_width_auto;
        for (const auto& [_, node] : nodes) {
            aabb.extend(node.position);
        }
    } else {
        border_width = border_width_explicit;
        for (const auto& p : bounding_contour) {
            aabb.extend(p);
        }
    }
}

BoundingInfo::BoundingInfo(
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& aabb,
    CompressedScenePos border_width,
    CompressedScenePos segment_length)
    : aabb{ aabb }
    , border_width{ border_width }
    , segment_length { segment_length }
{}
