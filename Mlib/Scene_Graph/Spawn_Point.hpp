#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
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
    TransformationMatrix<SceneDir, CompressedScenePos, 3> trafo = uninitialized;
    std::string team;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(type);
        archive(location);
        archive(trafo);
        archive(team);
    }
};

}
