#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;
class MacroLineExecutor;
struct LoadSceneJsonUserFunctionArgs;

class CreatePlayer: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreatePlayer(
        PhysicsScene& physics_scene,
        const MacroLineExecutor& macro_line_executor);
    void execute(const JsonView& args);
};

}
