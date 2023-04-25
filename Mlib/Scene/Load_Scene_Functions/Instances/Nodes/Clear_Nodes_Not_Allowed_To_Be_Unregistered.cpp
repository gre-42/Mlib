#include "Clear_Nodes_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

const std::string ClearNodesNotAllowedToBeUnregistered::key = "clear_nodes_not_allowed_to_be_unregistered";

LoadSceneJsonUserFunction ClearNodesNotAllowedToBeUnregistered::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    ClearNodesNotAllowedToBeUnregistered(args.renderable_scene()).execute(args);
};

ClearNodesNotAllowedToBeUnregistered::ClearNodesNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearNodesNotAllowedToBeUnregistered::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.clear_nodes_not_allowed_to_be_unregistered();
}
