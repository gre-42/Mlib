#pragma once
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TDataX, class TDataY>
class Interp;

Interp<double, FixedArray<double, 3>> interpolated_contour(
    const std::list<FixedArray<double, 3>>& contour);

std::list<FixedArray<double, 3>> subdivided_contour(
    const std::list<FixedArray<double, 3>>& contour,
    double scale,
    double distance);

}
