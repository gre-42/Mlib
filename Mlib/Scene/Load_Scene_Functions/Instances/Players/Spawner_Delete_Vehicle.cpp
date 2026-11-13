#include "Spawner_Delete_Vehicle.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
}

SpawnerDeleteVehicle::SpawnerDeleteVehicle(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerDeleteVehicle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto spawner_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::spawner);
    vehicle_spawners.get(spawner_name).delete_vehicle();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "spawner_delete_vehicle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SpawnerDeleteVehicle(args.physics_scene()).execute(args);
            });
    }
} obj;

}
