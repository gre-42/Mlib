#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class PlayerSetCanSelectBestWeapon: public LoadPhysicsSceneInstanceFunction {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
private:
    explicit PlayerSetCanSelectBestWeapon(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
