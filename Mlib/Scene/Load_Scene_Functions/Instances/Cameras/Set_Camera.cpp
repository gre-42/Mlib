#include "Set_Camera.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

const std::string SetCamera::key = "set_camera";

LoadSceneJsonUserFunction SetCamera::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetCamera(args.renderable_scene()).execute(args);
};

SetCamera::SetCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCamera::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    selected_cameras.set_camera_node_name(args.arguments.at<std::string>(KnownArgs::name));
}
