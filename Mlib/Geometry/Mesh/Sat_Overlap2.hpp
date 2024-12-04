#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPosition>
struct CollisionRidgeSphere;
class IIntersectableMesh;

void get_overlap2(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere<CompressedScenePos>& e1,
    ScenePos max_keep_normal,
    ScenePos& min_overlap,
    FixedArray<SceneDir, 3>& normal);

}
