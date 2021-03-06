#include "Player_Set_Waypoint.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

LoadSceneUserFunction PlayerSetWaypoint::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_set_waypoint"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+position=([\\w+-.]*) ([\\w+-.]*) ([\\w+-.]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetWaypoint(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetWaypoint::PlayerSetWaypoint(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetWaypoint::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.get_player(match[PLAYER_NAME].str()).single_waypoint().set_waypoint({
        safe_stof(match[POSITION_X].str()),
        safe_stof(match[POSITION_Y].str()),
        safe_stof(match[POSITION_Z].str())});
}
