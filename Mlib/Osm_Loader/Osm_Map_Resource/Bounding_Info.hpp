#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;

struct BoundingInfo {
    BoundingInfo(
        const UUVector<FixedArray<double, 2>>& bounding_contour,
        const std::map<std::string, Node>& nodes,
        double border_width);
    FixedArray<double, 2> boundary_min;
    FixedArray<double, 2> boundary_max;
    double border_width;
};

}
