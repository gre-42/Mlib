#include "Player_Set_Can_Aim.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(VALUE);

LoadSceneUserFunction PlayerSetCanAim::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_can_aim"
        "\\s+player=([\\w+-.]+)"
        "\\s+value=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetCanAim(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetCanAim::PlayerSetCanAim(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetCanAim::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Player& player = players.get_player(match[PLAYER_NAME].str());
    player.set_can_aim(safe_stob(match[VALUE].str()));

}
