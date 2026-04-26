#include "Spawner_Set_Nodes.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
}

SpawnerSetNodes::SpawnerSetNodes(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetNodes::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references["vehicles"]
        .at(asset_id)
        .rp;
    auto prefixes = vars.database.at<std::vector<std::string>>("node_prefixes");
    auto suffix = args.arguments.at<std::string>(KnownArgs::suffix);
    std::list<std::unique_ptr<SceneVehicle, DeleteFromPool<SceneVehicle>>> vehicles;
    for (const auto& prefix : prefixes) {
        auto name = VariableAndHash<std::string>{prefix + suffix};
        DanglingBaseClassRef<SceneNode> node = scene.get_node(name, CURRENT_SOURCE_LOCATION);
        auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
        vehicles.push_back(global_object_pool.create_unique<SceneVehicle>(
            CURRENT_SOURCE_LOCATION,
            name,
            node,
            rb));
    }
    vehicle_spawners
        .get(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::spawner))
        .set_scene_vehicles(std::move(vehicles));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "spawner_set_nodes",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SpawnerSetNodes{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
