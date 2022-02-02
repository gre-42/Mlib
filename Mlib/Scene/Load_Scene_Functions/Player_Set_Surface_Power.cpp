#include "Player_Set_Surface_Power.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(FORWARD);
DECLARE_OPTION(BACKWARD);

LoadSceneInstanceFunction::UserFunction PlayerSetSurfacePower::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_set_surface_power"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+forward=([\\w+-.]+)"
        "\\s+backward=([\\w+-.]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetSurfacePower(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetSurfacePower::PlayerSetSurfacePower(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetSurfacePower::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    players.get_player(match[PLAYER_NAME].str()).set_surface_power(
    safe_stof(match[FORWARD].str()),
    safe_stof(match[BACKWARD].str()));
}
