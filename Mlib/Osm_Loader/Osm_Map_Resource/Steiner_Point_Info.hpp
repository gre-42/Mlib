#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

enum class SteinerPointType {
    STREET_NEIGHBOR,
    FOREST_OUTLINE,
    WALL,
    TREE_NODE
};

struct SteinerPointInfo {
    FixedArray<double, 3> position;
    SteinerPointType type;
};

}
