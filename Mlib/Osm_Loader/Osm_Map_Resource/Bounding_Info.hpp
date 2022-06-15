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
        const std::vector<FixedArray<double, 2>>& bounding_contour,
        const std::map<std::string, Node>& nodes,
        double border_width);
    FixedArray<double, 2> boundary_min;
    FixedArray<double, 2> boundary_max;
    double border_width;
};

}
