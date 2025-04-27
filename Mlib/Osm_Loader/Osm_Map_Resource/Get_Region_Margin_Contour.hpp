#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

std::list<FixedArray<CompressedScenePos, 2>> get_region_margin_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& region,
    CompressedScenePos width);

}
