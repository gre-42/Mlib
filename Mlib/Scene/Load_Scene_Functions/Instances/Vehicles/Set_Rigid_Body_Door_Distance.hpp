#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class SetRigidBodyDoorDistance: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetRigidBodyDoorDistance(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
