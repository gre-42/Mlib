#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

enum class WayPointLocation;

enum class SpawnPointType {
    SPAWN_LINE,
    ROAD,
    PARKING
};

struct SpawnPoint {
    SpawnPointType type;
    WayPointLocation location;
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
};

}
