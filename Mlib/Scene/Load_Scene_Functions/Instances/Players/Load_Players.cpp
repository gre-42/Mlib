#include "Load_Players.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Game_Logic/Late_Join_Player_Factory.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(json);
}

LoadPlayers::LoadPlayers(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void LoadPlayers::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (late_join_player_factory.has_value()) {
        THROW_OR_ABORT("late_join_player_factory already set");
    }
    late_join_player_factory.emplace(
        args.arguments.path(KnownArgs::json),
        args.macro_line_executor,
        args.asset_references,
        args.remote_sites);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "load_players",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                LoadPlayers(args.physics_scene()).execute(args);
            });
    }
} obj;

}
