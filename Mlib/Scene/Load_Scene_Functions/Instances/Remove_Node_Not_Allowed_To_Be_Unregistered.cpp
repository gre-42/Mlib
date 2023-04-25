#include "Remove_Node_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

const std::string RemoveNodeNotAllowedToBeUnregistered::key = "remove_node_not_allowed_to_be_unregistered";

LoadSceneJsonUserFunction RemoveNodeNotAllowedToBeUnregistered::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RemoveNodeNotAllowedToBeUnregistered(args.renderable_scene()).execute(args);
};

RemoveNodeNotAllowedToBeUnregistered::RemoveNodeNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RemoveNodeNotAllowedToBeUnregistered::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.remove_node_not_allowed_to_be_unregistered(args.arguments.at<std::string>(KnownArgs::name));
}
