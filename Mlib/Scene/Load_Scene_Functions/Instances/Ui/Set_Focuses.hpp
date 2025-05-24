#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class SetFocuses: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetFocuses(PhysicsScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
