#include "Player_Set_Can_Shoot.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
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

const std::string PlayerSetCanShoot::key = "set_can_shoot";

LoadSceneJsonUserFunction PlayerSetCanShoot::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetCanShoot(args.renderable_scene()).execute(args);
};

PlayerSetCanShoot::PlayerSetCanShoot(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetCanShoot::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Player& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    player.set_can_shoot(
        control_source_from_string(args.arguments.at<std::string>(KnownArgs::source)),
        args.arguments.at<bool>(KnownArgs::value));

}
