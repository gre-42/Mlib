#include "Delete_Root_Node.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
}

const std::string DeleteRootNode::key = "delete_root_node";

LoadSceneJsonUserFunction DeleteRootNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DeleteRootNode(args.physics_scene()).execute(args);
};

DeleteRootNode::DeleteRootNode(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void DeleteRootNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.delete_root_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name));
}
