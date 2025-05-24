#include "Create_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(team);
}

const std::string CreateSpawner::key = "spawner_create";

LoadSceneJsonUserFunction CreateSpawner::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateSpawner(args.physics_scene()).execute(args);
};

CreateSpawner::CreateSpawner(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateSpawner::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto suffix = args.arguments.try_at<std::string>(KnownArgs::suffix);
    vehicle_spawners.set(
        name,
        std::make_unique<VehicleSpawner>(
            scene,
            suffix.has_value() ? *suffix : '_' + name + scene.get_temporary_instance_suffix(),
            args.arguments.at<std::string>(KnownArgs::team)));
}
