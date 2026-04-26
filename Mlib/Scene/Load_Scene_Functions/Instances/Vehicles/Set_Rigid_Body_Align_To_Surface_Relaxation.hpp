#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class SetRigidBodyAlignToSurfaceRelaxation: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetRigidBodyAlignToSurfaceRelaxation(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
