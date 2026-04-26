#include "Follow_Node.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Follow_Movable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
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

FollowNode::FollowNode(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void FollowNode::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    Linker linker{ physics_engine.advance_times_ };
    auto follower = args.arguments.at(KnownArgs::follower);
    auto followed = args.arguments.at(KnownArgs::followed);
    DanglingBaseClassRef<SceneNode> follower_node = scene.get_node(follower, CURRENT_SOURCE_LOCATION);
    DanglingBaseClassRef<SceneNode> followed_node = scene.get_node(followed, CURRENT_SOURCE_LOCATION);
    auto distance = args.arguments.at<float>(KnownArgs::distance);
    auto follow_movable = global_object_pool.create_unique<FollowMovable>(
        CURRENT_SOURCE_LOCATION,
        followed_node,
        followed_node->get_absolute_movable(CURRENT_SOURCE_LOCATION),
        distance,
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::node_displacement),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::look_at_displacement),
        args.arguments.at<float>(KnownArgs::snappiness),
        args.arguments.at<float>(KnownArgs::y_adaptivity),
        args.arguments.at<float>(KnownArgs::y_snappiness),
        scene_config.physics_engine_config.dt);
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
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "follow_node",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                FollowNode{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
