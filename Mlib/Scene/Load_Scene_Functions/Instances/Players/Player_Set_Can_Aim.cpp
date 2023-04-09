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
DECLARE_OPTION(SOURCE);
DECLARE_OPTION(VALUE);

const std::string PlayerSetCanAim::key = "set_can_aim";

LoadSceneUserFunction PlayerSetCanAim::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^player=([\\w+-.]+)"
        "\\s+source=(\\w+)"
        "\\s+value=(0|1)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    PlayerSetCanAim(args.renderable_scene()).execute(match, args);
};

PlayerSetCanAim::PlayerSetCanAim(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetCanAim::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Player& player = players.get_player(match[PLAYER_NAME].str());
    player.set_can_aim(control_source_from_string(match[SOURCE].str()), safe_stob(match[VALUE].str()));

}
