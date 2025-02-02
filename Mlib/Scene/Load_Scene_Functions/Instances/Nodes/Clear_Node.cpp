#include "Clear_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

const std::string ClearNode::key = "clear_node";

LoadSceneJsonUserFunction ClearNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearNode(args.renderable_scene()).execute(args);
};

ClearNode::ClearNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC)->clear();
}
