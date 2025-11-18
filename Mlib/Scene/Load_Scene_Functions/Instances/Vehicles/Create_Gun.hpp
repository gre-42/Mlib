#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;
class MacroLineExecutor;
struct LoadSceneJsonUserFunctionArgs;

class CreateGun: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreateGun(
        PhysicsScene& physics_scene,
        const MacroLineExecutor& macro_line_executor);
    void operator () (const JsonView& args);
};

}
