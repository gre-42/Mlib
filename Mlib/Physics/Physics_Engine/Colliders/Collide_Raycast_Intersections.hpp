#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionSceneAndContact;

void collide_raycast_intersections(
    const std::unordered_map<const FixedArray<FixedArray<ScenePos, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections);

}
