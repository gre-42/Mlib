#include "Create_Player.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(game_mode);
DECLARE_ARGUMENT(unstuck_mode);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(driving_direction);
}

const std::string CreatePlayer::key = "player_create";

LoadSceneJsonUserFunction CreatePlayer::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlayer(args.renderable_scene()).execute(args);
};

CreatePlayer::CreatePlayer(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlayer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Game logic is null, cannot create player");
    }
    auto player = std::make_unique<Player>(
        scene,
        supply_depots,
        game_logic->navigate,
        scene_config.physics_engine_config,
        physics_engine.collision_query_,
        vehicle_spawners,
        players,
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<std::string>(KnownArgs::team),
        game_mode_from_string(args.arguments.at<std::string>(KnownArgs::game_mode)),
        unstuck_mode_from_string(args.arguments.at<std::string>(KnownArgs::unstuck_mode)),
        args.arguments.at<std::string>(KnownArgs::behavior),
        driving_direction_from_string(args.arguments.at<std::string>(KnownArgs::driving_direction)),
        delete_node_mutex,
        args.ui_focus.focuses);
    players.add_player({ *player, CURRENT_SOURCE_LOCATION });
    auto& p = global_object_pool.add(std::move(player), CURRENT_SOURCE_LOCATION);
    physics_engine.advance_times_.add_advance_time({ p, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    physics_engine.add_external_force_provider(p);
}
