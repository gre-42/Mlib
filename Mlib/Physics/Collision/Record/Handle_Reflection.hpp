#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;

void handle_reflection(
    const IntersectionScene& c,
    const FixedArray<ScenePos, 3>& intersection_point,
    float surface_stiction_factor);

}
