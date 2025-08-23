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
    auto follower = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower);
    auto followed = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::followed);
    DanglingRef<SceneNode> follower_node = scene.get_node(follower, DP_LOC);
    DanglingRef<SceneNode> followed_node = scene.get_node(followed, DP_LOC);
    auto look_at = global_object_pool.create_unique<LookAtMovable>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        scene,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower),
        follower_node,
        followed_node,
        followed_node->get_absolute_movable());
    linker.link_absolute_movable_and_additional_node(
        scene,
        std::move(follower_node),
        std::move(followed_node),
        look_at->follower_setter,
        look_at->followed_setter,
        std::move(follower),
        std::move(followed),
        std::move(look_at),
        CURRENT_SOURCE_LOCATION);
}
