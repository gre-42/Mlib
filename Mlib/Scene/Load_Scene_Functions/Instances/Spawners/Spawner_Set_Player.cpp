#include "Spawner_Set_Player.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(seat);
}

const std::string SpawnerSetPlayer::key = "spawner_set_player";

LoadSceneJsonUserFunction SpawnerSetPlayer::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SpawnerSetPlayer(args.physics_scene()).execute(args);
};

SpawnerSetPlayer::SpawnerSetPlayer(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetPlayer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
    vehicle_spawners.get(name).set_player(
        players.get_player(name, CURRENT_SOURCE_LOCATION),
        args.arguments.at<std::string>(KnownArgs::seat));
}
