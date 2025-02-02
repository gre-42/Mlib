#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

enum class SteinerPointType {
    STREET_NEIGHBOR,
    FOREST_OUTLINE,
    WALL,
    TREE_NODE
};

struct SteinerPointInfo {
    FixedArray<CompressedScenePos, 3> position;
    SteinerPointType type;
};

}
