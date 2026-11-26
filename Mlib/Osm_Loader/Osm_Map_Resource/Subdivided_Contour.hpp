#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class ContourClosedness {
    OPEN_OR_CLOSED_WITH_LAST_ELEMENT,
    CLOSED_WO_LAST_ELEMENT
};

template <size_t tndim>
UUInterp<double, FixedArray<double, tndim>> interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    ContourClosedness contour_closedness);

template <size_t tndim>
std::list<FixedArray<CompressedScenePos, tndim>> subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    double scale,
    CompressedScenePos distance,
    ContourClosedness contour_closedness);

}
