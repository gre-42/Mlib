#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

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
    std::string group;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(type);
        archive(location);
        archive(trafo);
        archive(team);
        archive(group);
    }
};

}
