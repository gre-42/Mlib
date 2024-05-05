#include "Player_Set_Can_Select_Weapon.hpp"
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

const std::string PlayerSetCanSelectBestWeapon::key = "set_can_select_weapon";

LoadSceneJsonUserFunction PlayerSetCanSelectBestWeapon::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetCanSelectBestWeapon(args.renderable_scene()).execute(args);
};

PlayerSetCanSelectBestWeapon::PlayerSetCanSelectBestWeapon(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetCanSelectBestWeapon::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->set_can_select_weapon(
        control_source_from_string(args.arguments.at<std::string>(KnownArgs::source)),
        args.arguments.at<bool>(KnownArgs::value));

}
