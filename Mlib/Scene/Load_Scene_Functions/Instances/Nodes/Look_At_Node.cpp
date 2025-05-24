#include "Look_At_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Look_At_Movable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(follower);
DECLARE_ARGUMENT(followed);
}

const std::string LookAtNode::key = "look_at_node";

LoadSceneJsonUserFunction LookAtNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    LookAtNode(args.physics_scene()).execute(args);
};

LookAtNode::LookAtNode(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void LookAtNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> follower_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower), DP_LOC);
    DanglingRef<SceneNode> followed_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::followed), DP_LOC);
    auto follower = global_object_pool.create_unique<LookAtMovable>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        scene,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower),
        follower_node,
        followed_node,
        followed_node->get_absolute_movable());
    linker.link_absolute_movable_and_additional_node(
        follower_node,
        followed_node,
        std::move(follower),
        CURRENT_SOURCE_LOCATION);
}
