#include "Spawner_Set_Respawn_Cooldown_Time.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(time);
}

const std::string SpawnerSetRespawnCooldownTime::key = "set_respawn_cooldown_time";

LoadSceneJsonUserFunction SpawnerSetRespawnCooldownTime::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SpawnerSetRespawnCooldownTime(args.physics_scene()).execute(args);
};

SpawnerSetRespawnCooldownTime::SpawnerSetRespawnCooldownTime(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetRespawnCooldownTime::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    vehicle_spawners
        .get(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::spawner))
        .set_respawn_cooldown_time(args.arguments.at<float>(KnownArgs::time) * seconds);
}
