#include "Follow_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Follow_Movable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(follower);
DECLARE_ARGUMENT(followed);
DECLARE_ARGUMENT(distance);
DECLARE_ARGUMENT(node_displacement);
DECLARE_ARGUMENT(look_at_displacement);
DECLARE_ARGUMENT(snappiness);
DECLARE_ARGUMENT(y_adaptivity);
DECLARE_ARGUMENT(y_snappiness);
}

const std::string FollowNode::key = "follow_node";

LoadSceneJsonUserFunction FollowNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    FollowNode(args.physics_scene()).execute(args);
};

FollowNode::FollowNode(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void FollowNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto follower = args.arguments.at(KnownArgs::follower);
    auto followed = args.arguments.at(KnownArgs::followed);
    DanglingBaseClassRef<SceneNode> follower_node = scene.get_node(follower, DP_LOC);
    DanglingBaseClassRef<SceneNode> followed_node = scene.get_node(followed, DP_LOC);
    auto distance = args.arguments.at<float>(KnownArgs::distance);
    auto follow_movable = global_object_pool.create_unique<FollowMovable>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        followed_node,
        followed_node->get_absolute_movable(),
        distance,
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::node_displacement),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::look_at_displacement),
        args.arguments.at<float>(KnownArgs::snappiness),
        args.arguments.at<float>(KnownArgs::y_adaptivity),
        args.arguments.at<float>(KnownArgs::y_snappiness),
        scene_config.physics_engine_config.dt);
    auto& follow_movable_p = *follow_movable;
    linker.link_absolute_movable_and_additional_node(
        scene,
        follower_node,
        followed_node,
        follow_movable->set_follower,
        follow_movable->set_followed,
        follower,
        followed,
        std::move(follow_movable),
        CURRENT_SOURCE_LOCATION);
    follow_movable_p.initialize(follower_node);
}
