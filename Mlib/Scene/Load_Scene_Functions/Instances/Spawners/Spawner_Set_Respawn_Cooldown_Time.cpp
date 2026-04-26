#include "Spawner_Set_Respawn_Cooldown_Time.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(time);
}

SpawnerSetRespawnCooldownTime::SpawnerSetRespawnCooldownTime(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetRespawnCooldownTime::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    vehicle_spawners
        .get(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::spawner))
        .set_respawn_cooldown_time(args.arguments.at<float>(KnownArgs::time) * seconds);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_respawn_cooldown_time",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SpawnerSetRespawnCooldownTime{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
