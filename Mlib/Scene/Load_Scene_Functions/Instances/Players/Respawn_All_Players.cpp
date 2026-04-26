#include "Respawn_All_Players.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

RespawnAllPlayers::RespawnAllPlayers(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RespawnAllPlayers::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    if (game_logic == nullptr) {
        throw std::runtime_error("Scene has no game logic");
    }
    game_logic->spawner.respawn_all_players();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "respawn_all_players",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                RespawnAllPlayers{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
