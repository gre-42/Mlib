#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

UUInterp<double, FixedArray<double, 3>> interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour);

std::list<FixedArray<CompressedScenePos, 3>> subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour,
    double scale,
    CompressedScenePos distance);

}
