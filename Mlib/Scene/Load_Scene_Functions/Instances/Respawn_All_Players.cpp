#include "Respawn_All_Players.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction RespawnAllPlayers::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*respawn_all_players$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RespawnAllPlayers(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RespawnAllPlayers::RespawnAllPlayers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RespawnAllPlayers::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    game_logic.spawn.respawn_all_players();
}
