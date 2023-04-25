#include "Clear_Node_Hider.hpp"
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

const std::string ClearNodeHider::key = "clear_node_hider";

LoadSceneJsonUserFunction ClearNodeHider::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearNodeHider(args.renderable_scene()).execute(args);
};

ClearNodeHider::ClearNodeHider(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearNodeHider::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.get_node(args.arguments.at<std::string>(KnownArgs::node)).clear_node_hider();
}
