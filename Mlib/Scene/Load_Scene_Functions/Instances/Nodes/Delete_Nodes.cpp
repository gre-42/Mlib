#include "Delete_Nodes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(regex);
}

const std::string DeleteNodes::key = "delete_nodes";

LoadSceneJsonUserFunction DeleteNodes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DeleteNodes(args.renderable_scene()).execute(args);
};

DeleteNodes::DeleteNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteNodes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.delete_nodes(Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::regex)));
}
