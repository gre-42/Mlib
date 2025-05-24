#include "Delete_Root_Nodes.hpp"
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

const std::string DeleteRootNodes::key = "delete_root_nodes";

LoadSceneJsonUserFunction DeleteRootNodes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DeleteRootNodes(args.physics_scene()).execute(args);
};

DeleteRootNodes::DeleteRootNodes(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void DeleteRootNodes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.delete_root_nodes(Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::regex)));
}
