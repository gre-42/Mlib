#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;

struct BoundingInfo {
    BoundingInfo(
        const std::vector<FixedArray<float, 2>>& bounding_contour,
        const std::map<std::string, Node>& nodes,
        float border_width);
    FixedArray<float, 2> boundary_min;
    FixedArray<float, 2> boundary_max;
    float border_width;
};

}
