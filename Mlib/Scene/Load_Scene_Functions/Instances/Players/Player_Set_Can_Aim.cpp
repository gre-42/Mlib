#include "Player_Set_Can_Aim.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(source);
DECLARE_ARGUMENT(value);
}

const std::string PlayerSetCanAim::key = "set_can_aim";

LoadSceneJsonUserFunction PlayerSetCanAim::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetCanAim(args.physics_scene()).execute(args);
};

PlayerSetCanAim::PlayerSetCanAim(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void PlayerSetCanAim::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->set_can_aim(
        control_source_from_string(args.arguments.at<std::string>(KnownArgs::source)),
        args.arguments.at<bool>(KnownArgs::value));

}
