#include "Move_Node_To_Bvh.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

const std::string MoveNodeToBvh::key = "move_node_to_bvh";

LoadSceneJsonUserFunction MoveNodeToBvh::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    MoveNodeToBvh(args.physics_scene()).execute(args);
};

MoveNodeToBvh::MoveNodeToBvh(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void MoveNodeToBvh::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.move_root_node_to_bvh(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node));
}
