#include "Respawn_All_Players.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>

using namespace Mlib;

const std::string RespawnAllPlayers::key = "respawn_all_players";

LoadSceneUserFunction RespawnAllPlayers::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    RespawnAllPlayers(args.renderable_scene()).execute(match, args);
};

RespawnAllPlayers::RespawnAllPlayers(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RespawnAllPlayers::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    game_logic.spawn.respawn_all_players();
}
