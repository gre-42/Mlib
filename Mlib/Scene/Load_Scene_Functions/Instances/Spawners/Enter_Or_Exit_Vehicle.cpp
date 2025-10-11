#include "Enter_Or_Exit_Vehicle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Vehicle_Changer.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(destination);
DECLARE_ARGUMENT(seat);
}

EnterOrExitVehicle::EnterOrExitVehicle(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void EnterOrExitVehicle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Game logic not set");
    }
    auto player_name = args.arguments.at<std::string>(KnownArgs::player);
    auto destination_name = args.arguments.at<std::string>(KnownArgs::destination);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& destination = vehicle_spawners.get(destination_name);
    auto seat = args.arguments.at<std::string>(KnownArgs::seat);
    player->set_next_vehicle(destination, destination.get_primary_scene_vehicle().get(), seat);
    if (!game_logic->vehicle_changer.change_vehicle(player->vehicle_spawner().get())) {
        THROW_OR_ABORT("Player \"" + player_name + "\" could not enter \"" + destination_name + '"');
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "enter_or_exit_vehicle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                EnterOrExitVehicle(args.physics_scene()).execute(args);            
            });
    }
} obj;

}
