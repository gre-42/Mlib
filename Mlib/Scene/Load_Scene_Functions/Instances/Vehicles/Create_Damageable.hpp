#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;
struct LoadSceneJsonUserFunctionArgs;

class CreateDamageable: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreateDamageable(PhysicsScene& physics_scene);
    void execute(const JsonView& args);
};

}
