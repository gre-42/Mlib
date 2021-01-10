#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

enum class SpawnPointType {
    SPAWN_LINE,
    ROAD,
    PARKING,
    SIDEWALK
};

struct SpawnPoint {
    SpawnPointType type;
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
};

}
