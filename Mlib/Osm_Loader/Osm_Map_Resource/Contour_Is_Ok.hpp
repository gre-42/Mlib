#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

bool contour_is_ok(
    const std::vector<FixedArray<CompressedScenePos, 2>>& contour,
    CompressedScenePos min_distance);
    
}
