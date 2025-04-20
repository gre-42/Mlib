#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <string>

namespace Mlib {

enum class SpawnAction {
    DRY_RUN,
    DO_IT
};

struct GeometrySpawnArguments {
    SpawnAction action;
};

struct NodeSpawnArguments {
    std::string suffix;
    bool if_with_graphics;
    bool if_with_physics;
};

}
