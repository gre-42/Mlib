#include "Create_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
}

const std::string CreateSpawner::key = "spawner_create";

LoadSceneJsonUserFunction CreateSpawner::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateSpawner(args.renderable_scene()).execute(args);
};

CreateSpawner::CreateSpawner(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateSpawner::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    vehicle_spawners.set(
        args.arguments.at<std::string>(KnownArgs::name),
        std::make_unique<VehicleSpawner>(
            scene,
            args.arguments.at<std::string>(KnownArgs::team)));
}
