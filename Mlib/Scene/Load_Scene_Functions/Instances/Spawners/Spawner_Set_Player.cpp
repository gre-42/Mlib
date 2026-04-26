#include "Spawner_Set_Player.hpp"
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(seat);
}

SpawnerSetPlayer::SpawnerSetPlayer(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetPlayer::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
    vehicle_spawners.get(name).set_player(
        players.get_player(name, CURRENT_SOURCE_LOCATION),
        args.arguments.at<std::string>(KnownArgs::seat));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "spawner_set_player",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SpawnerSetPlayer{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
