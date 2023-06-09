#include "Create_Camera_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
}

const std::string CreateCameraKeyBinding::key = "camera_key_binding";

LoadSceneJsonUserFunction CreateCameraKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCameraKeyBinding(args.renderable_scene()).execute(args);
};

CreateCameraKeyBinding::CreateCameraKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCameraKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    key_bindings.add_camera_key_binding({.id = args.arguments.at<std::string>(KnownArgs::id)});
}
