#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonView;
class MacroLineExecutor;
class SceneVehicle;
struct LoadSceneJsonUserFunctionArgs;

class SetExternalsCreator: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetExternalsCreator(
        PhysicsScene& physics_scene,
        const MacroLineExecutor& macro_line_executor);
    void execute_safe(
        SceneVehicle& vehicle,
        const std::string& asset_id);
    void execute_safe(const LoadSceneJsonUserFunctionArgs& args);
    void execute_unsafe(const LoadSceneJsonUserFunctionArgs& args);
private:
    void execute_unsafe(
        SceneVehicle& vehicle,
        nlohmann::json externals,
        nlohmann::json internals);
};

}
