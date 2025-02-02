#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>

namespace Mlib {

class SceneNode;

struct AvatarControllerIdleBinding {
    DanglingPtr<SceneNode> node;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
