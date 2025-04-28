#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct Node;
template <typename TData, size_t... tshape>
class FixedArray;

struct SubdividedWayVertex {
    const Node& n0;
    const Node& n1;
    ScenePos a0;
    ScenePos a1;
    FixedArray<CompressedScenePos, 2> position() const;
};

}
