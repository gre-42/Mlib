#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionSceneAndContact;

void collide_raycast_intersections(
    const std::unordered_map<const FixedArray<ScenePos, 2, 3>*, IntersectionSceneAndContact>& raycast_intersections);

}
