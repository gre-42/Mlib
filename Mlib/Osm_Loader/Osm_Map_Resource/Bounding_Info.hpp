#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;

struct BoundingInfo {
    BoundingInfo(
        const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
        const std::map<std::string, Node>& nodes,
        CompressedScenePos border_width_explicit,
        CompressedScenePos border_width_auto,
        CompressedScenePos segment_length);
    BoundingInfo(
        const AxisAlignedBoundingBox<CompressedScenePos, 2>& aabb,
        CompressedScenePos border_width,
        CompressedScenePos segment_length);
    AxisAlignedBoundingBox<CompressedScenePos, 2> aabb;
    CompressedScenePos border_width;
    CompressedScenePos segment_length;
};

}
