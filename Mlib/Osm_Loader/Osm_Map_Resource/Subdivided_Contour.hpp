#pragma once
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

std::list<FixedArray<double, 3>> subdivided_contour(
    const std::list<FixedArray<double, 3>>& contour,
    double scale,
    double distance);

}
