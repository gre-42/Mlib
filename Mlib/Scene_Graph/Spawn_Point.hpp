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
    FixedArray<double, 3> position;
    FixedArray<float, 3> rotation;
    std::string team;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(type);
        archive(location);
        archive(position);
        archive(rotation);
        archive(team);
    }
};

}
