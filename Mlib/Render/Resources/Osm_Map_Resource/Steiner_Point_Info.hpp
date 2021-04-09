#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class SteinerPointType {
    STREET_NEIGHBOR,
    FOREST_OUTLINE,
    WALL,
    TREE_NODE,
    MODEL_NODE
};

struct SteinerPointInfo {
    FixedArray<float, 3> position;
    SteinerPointType type;
    float distance_to_road;
    float distance_to_air_road;
};

}
