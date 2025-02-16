#include "Reload_Scene.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(change_scene);
}

const std::string ReloadScene::key = "reload_scene";

LoadSceneJsonUserFunction ReloadScene::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ReloadScene::execute(args);
};

void ReloadScene::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (!args.arguments.at<bool>(KnownArgs::change_scene)) {
        args.next_scene_filename = "";
    }
    args.num_renderings = 0;
}
