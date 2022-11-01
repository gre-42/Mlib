#include "Player_Set_Can_Select_Best_Weapon.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(SOURCE);
DECLARE_OPTION(VALUE);

LoadSceneUserFunction PlayerSetCanSelectBestWeapon::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_can_select_best_weapon"
        "\\s+player=([\\w+-.]+)"
        "\\s+source=(\\w+)"
        "\\s+value=(0|1)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetCanSelectBestWeapon(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetCanSelectBestWeapon::PlayerSetCanSelectBestWeapon(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetCanSelectBestWeapon::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Player& player = players.get_player(match[PLAYER_NAME].str());
    player.set_can_select_best_weapon(control_source_from_string(match[SOURCE].str()), safe_stob(match[VALUE].str()));

}
