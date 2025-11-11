#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;
class MacroLineExecutor;
struct LoadSceneJsonUserFunctionArgs;

class SetExternalsCreator: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetExternalsCreator(PhysicsScene& physics_scene);
    void execute_safe(
        const std::string& spawner,
        const std::string& asset_id,
        const MacroLineExecutor& macro_line_executor);
    void execute_safe(const LoadSceneJsonUserFunctionArgs& args);
    void execute_unsafe(const LoadSceneJsonUserFunctionArgs& args);
private:
    void execute_unsafe(
        const JsonView& arguments,
        const MacroLineExecutor& macro_line_executor);
};

}
