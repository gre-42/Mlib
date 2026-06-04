#include "Create_Spawner.hpp"
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(group);
DECLARE_ARGUMENT(trigger);
}

namespace KnownNodeArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
}

CreateSpawner::CreateSpawner(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateSpawner::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
    auto get_node = [&](){
        auto j = args.arguments.at(KnownArgs::node);
        JsonView jv{j};
        jv.validate(KnownNodeArgs::options);
        auto suffix = args.arguments.try_at<std::string>(KnownNodeArgs::suffix);
        return NodeSpawnArguments{
            suffix.has_value() ? *suffix : '_' + *name + scene.get_temporary_instance_suffix(),
            jv.at<bool>(KnownNodeArgs::if_with_graphics),
            jv.at<bool>(KnownNodeArgs::if_with_physics)
        };
    };
    vehicle_spawners.set(
        name,
        std::make_unique<VehicleSpawner>(
            scene,
            get_node(),
            args.arguments.at<std::string>(KnownArgs::team),
            args.arguments.at<std::string>(KnownArgs::group),
            spawn_trigger_from_string(args.arguments.at<std::string>(KnownArgs::trigger))));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "spawner_create",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateSpawner{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
