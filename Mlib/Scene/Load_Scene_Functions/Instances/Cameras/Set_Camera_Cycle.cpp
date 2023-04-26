#include "Set_Camera_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
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
    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto cameras = args.arguments.at_non_null<std::vector<std::string>>(KnownArgs::cameras, {});
    if (name == "near") {
        selected_cameras.set_camera_cycle_near(cameras);
    } else if (name == "far") {
        selected_cameras.set_camera_cycle_far(cameras);
    } else {
        THROW_OR_ABORT("Unknown camera cycle");
    }
}
