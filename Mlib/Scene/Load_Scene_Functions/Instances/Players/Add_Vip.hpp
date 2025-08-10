#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class AddVip: public LoadPhysicsSceneInstanceFunction {
public:
    explicit AddVip(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
