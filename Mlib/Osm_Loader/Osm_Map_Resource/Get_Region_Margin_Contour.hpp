#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <typename TData, size_t... tshape>
class OrderableFixedArray;
struct SubdividedWayVertex;

std::list<FixedArray<CompressedScenePos, 2>> get_region_margin_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& region,
    CompressedScenePos width,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos>& garden_margin);

}
