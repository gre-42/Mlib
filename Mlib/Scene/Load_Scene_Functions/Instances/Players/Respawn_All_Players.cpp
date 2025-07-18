#include "Respawn_All_Players.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string RespawnAllPlayers::key = "respawn_all_players";

LoadSceneJsonUserFunction RespawnAllPlayers::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate({});
    RespawnAllPlayers(args.physics_scene()).execute(args);
};

RespawnAllPlayers::RespawnAllPlayers(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RespawnAllPlayers::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Scene has no game logic");
    }
    game_logic->spawner.respawn_all_players();
}
