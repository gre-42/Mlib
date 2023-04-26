#include "Player_Set_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
}

const std::string PlayerSetNode::key = "player_set_node";

LoadSceneJsonUserFunction PlayerSetNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetNode(args.renderable_scene()).execute(args);
};

PlayerSetNode::PlayerSetNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Follower movable is not a rigid body");
    }
    players.get_player(args.arguments.at<std::string>(KnownArgs::player)).set_rigid_body(
        PlayerVehicle{
            .scene_node_name = args.arguments.at<std::string>(KnownArgs::node),
            .scene_node = &node,
            .rb = rb});
}
