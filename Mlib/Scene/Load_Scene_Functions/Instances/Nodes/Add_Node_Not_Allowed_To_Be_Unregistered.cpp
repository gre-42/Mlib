#include "Add_Node_Not_Allowed_To_Be_Unregistered.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

const std::string AddNodeNotAllowedToBeUnregistered::key = "add_node_not_allowed_to_be_unregistered";

LoadSceneJsonUserFunction AddNodeNotAllowedToBeUnregistered::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    AddNodeNotAllowedToBeUnregistered(args.renderable_scene()).execute(args);
};

AddNodeNotAllowedToBeUnregistered::AddNodeNotAllowedToBeUnregistered(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void AddNodeNotAllowedToBeUnregistered::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.add_node_not_allowed_to_be_unregistered(args.arguments.at<std::string>(KnownArgs::name));
}
