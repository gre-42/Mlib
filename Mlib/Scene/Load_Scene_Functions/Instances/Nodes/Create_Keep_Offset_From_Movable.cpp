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
    DanglingRef<SceneNode> follower_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower), DP_LOC);
    DanglingRef<SceneNode> followed_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::followed), DP_LOC);
    auto follower = global_object_pool.create_unique<KeepOffsetFromMovable>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        scene,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::follower),
        followed_node,
        followed_node->get_absolute_movable(),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::offset));
    linker.link_absolute_movable(follower_node, std::move(follower), CURRENT_SOURCE_LOCATION);
}
