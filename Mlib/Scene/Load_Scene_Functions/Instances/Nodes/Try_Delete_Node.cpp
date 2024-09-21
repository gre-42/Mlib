#include "Try_Delete_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

const std::string TryDeleteNode::key = "try_delete_node";

LoadSceneJsonUserFunction TryDeleteNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    TryDeleteNode(args.renderable_scene()).execute(args);
};

TryDeleteNode::TryDeleteNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void TryDeleteNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.try_delete_node(args.arguments.at<std::string>(KnownArgs::name));
}
