#include "Create_Keep_Offset_From_Movable.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Keep_Offset_From_Movable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(follower);
DECLARE_ARGUMENT(followed);
DECLARE_ARGUMENT(offset);
}

const std::string CreateKeepOffsetFromMovable::key = "keep_offset_from_movable";

LoadSceneJsonUserFunction CreateKeepOffsetFromMovable::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateKeepOffsetFromMovable(args.physics_scene()).execute(args);
};

CreateKeepOffsetFromMovable::CreateKeepOffsetFromMovable(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateKeepOffsetFromMovable::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto follower = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower);
    auto followed = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::followed);
    DanglingBaseClassRef<SceneNode> follower_node = scene.get_node(follower, DP_LOC);
    DanglingBaseClassRef<SceneNode> followed_node = scene.get_node(followed, DP_LOC);
    auto keep_offset = global_object_pool.create_unique<KeepOffsetFromMovable>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        scene,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower),
        followed_node,
        followed_node->get_absolute_movable(),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::offset));
    linker.link_absolute_movable_and_additional_node(
        scene,
        follower_node,
        followed_node,
        keep_offset->set_follower,
        keep_offset->set_followed,
        follower,
        followed,
        std::move(keep_offset),
        CURRENT_SOURCE_LOCATION);
}
