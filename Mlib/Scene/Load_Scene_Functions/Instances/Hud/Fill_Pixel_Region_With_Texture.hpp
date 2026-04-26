#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class FillPixelRegionWithTexture: public LoadPhysicsSceneInstanceFunction {
public:
    explicit FillPixelRegionWithTexture(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
