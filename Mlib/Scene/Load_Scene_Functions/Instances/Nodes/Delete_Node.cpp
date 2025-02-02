#include "Delete_Node.hpp"
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

const std::string DeleteNode::key = "delete_node";

LoadSceneJsonUserFunction DeleteNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DeleteNode(args.renderable_scene()).execute(args);
};

DeleteNode::DeleteNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DeleteNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.delete_node(args.arguments.at<std::string>(KnownArgs::name));
}
