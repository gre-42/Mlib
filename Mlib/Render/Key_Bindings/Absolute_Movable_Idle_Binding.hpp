#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct AbsoluteMovableIdleBinding {
    DanglingPtr<SceneNode> node;
    FixedArray<float, 3> tires_z;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
