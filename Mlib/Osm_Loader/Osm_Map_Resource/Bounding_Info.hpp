#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
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
        CompressedScenePos border_width,
        CompressedScenePos segment_length);
    FixedArray<CompressedScenePos, 2> boundary_min;
    FixedArray<CompressedScenePos, 2> boundary_max;
    CompressedScenePos border_width;
    CompressedScenePos segment_length;
};

}
