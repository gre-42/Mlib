#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <size_t tndim>
UUInterp<double, FixedArray<double, tndim>> interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour);

template <size_t tndim>
std::list<FixedArray<CompressedScenePos, tndim>> subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    double scale,
    CompressedScenePos distance);

}
