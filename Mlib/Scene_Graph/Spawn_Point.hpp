#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

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
    FixedArray<CompressedScenePos, 3> position = uninitialized;
    FixedArray<float, 3> rotation = uninitialized;
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
