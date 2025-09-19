#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>

namespace Mlib {

class SceneNode;

struct CarControllerIdleBinding {
    DanglingBaseClassPtr<SceneNode> node;
    float surface_power;
    float steer_angle;
    float drive_relaxation;
    float steer_relaxation;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
