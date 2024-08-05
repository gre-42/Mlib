#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionRidgeSphere;
class IIntersectableMesh;

void get_overlap2(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere& e1,
    ScenePos max_keep_normal,
    ScenePos& min_overlap,
    FixedArray<ScenePos, 3>& normal);

}
