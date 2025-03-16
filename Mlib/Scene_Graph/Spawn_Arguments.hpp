#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <string>

namespace Mlib {

struct SpawnArguments {
    std::string suffix;
    bool if_with_graphics;
    bool if_with_physics;
    CompressedScenePos y_offset;
};

}
