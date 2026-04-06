#include "Set_Camera_Cycle.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(cameras);
}

SetCameraCycle::SetCameraCycle(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void SetCameraCycle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto tpe = camera_cycle_type_from_string(args.arguments.at<std::string>(KnownArgs::name));
    auto cameras = args.arguments.at_non_null<std::vector<VariableAndHash<std::string>>>(KnownArgs::cameras, {});
    selected_cameras.set_camera_cycle(tpe, cameras);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_camera_cycle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetCameraCycle(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
