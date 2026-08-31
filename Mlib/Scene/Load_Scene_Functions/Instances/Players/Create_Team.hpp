#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;

class CreateTeam: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreateTeam(PhysicsScene& physics_scene);
    void execute(const JsonView& args);
};

}
