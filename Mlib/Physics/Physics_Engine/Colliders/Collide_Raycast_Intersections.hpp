#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class OrderableFixedArray;
struct IntersectionSceneAndContact;

void collide_raycast_intersections(
    const std::unordered_map<OrderableFixedArray<CompressedScenePos, 2, 3>, IntersectionSceneAndContact>& raycast_intersections);

}
