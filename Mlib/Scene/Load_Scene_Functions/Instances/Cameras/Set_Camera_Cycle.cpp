#include "Set_Camera_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(cameras);
}

const std::string SetCameraCycle::key = "set_camera_cycle";

LoadSceneJsonUserFunction SetCameraCycle::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetCameraCycle(args.renderable_scene()).execute(args);
};

SetCameraCycle::SetCameraCycle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCameraCycle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto tpe = camera_cycle_type_from_string(args.arguments.at<std::string>(KnownArgs::name));
    auto cameras = args.arguments.at_non_null<std::vector<VariableAndHash<std::string>>>(KnownArgs::cameras, {});
    selected_cameras.set_camera_cycle(tpe, cameras);
}
