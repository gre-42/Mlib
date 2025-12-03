#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;
enum class UserTypes;

class ForEachSiteUser {
public:
    ForEachSiteUser(UserTypes user_type);
    ~ForEachSiteUser();
    void execute(const LoadSceneJsonUserFunctionArgs& args);
private:
    UserTypes user_types_;
};

class OnUserLoadedLevel: public LoadPhysicsSceneInstanceFunction {
public:
    OnUserLoadedLevel(PhysicsScene& physics_scene);
    ~OnUserLoadedLevel();
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
