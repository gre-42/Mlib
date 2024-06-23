#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

UUInterp<double, FixedArray<double, 3>> interpolated_contour(
    const std::list<FixedArray<double, 3>>& contour);

std::list<FixedArray<double, 3>> subdivided_contour(
    const std::list<FixedArray<double, 3>>& contour,
    double scale,
    double distance);

}
