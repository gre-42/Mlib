#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class YplnUpdateBulletProperties: public LoadPhysicsSceneInstanceFunction {
public:
    explicit YplnUpdateBulletProperties(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
