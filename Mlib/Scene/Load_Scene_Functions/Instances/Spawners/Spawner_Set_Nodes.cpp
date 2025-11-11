#include "Spawner_Set_Nodes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
}

const std::string SpawnerSetNodes::key = "spawner_set_nodes";

LoadSceneJsonUserFunction SpawnerSetNodes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SpawnerSetNodes(args.physics_scene()).execute(args);
};

SpawnerSetNodes::SpawnerSetNodes(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SpawnerSetNodes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
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
        DanglingBaseClassRef<SceneNode> node = scene.get_node(name, DP_LOC);
        auto& rb = get_rigid_body_vehicle(node);
        vehicles.push_back(global_object_pool.create_unique<SceneVehicle>(
            CURRENT_SOURCE_LOCATION,
            delete_node_mutex,
            name,
            node,
            DanglingBaseClassRef<RigidBodyVehicle>{ rb, CURRENT_SOURCE_LOCATION }));
    }
    vehicle_spawners
        .get(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::spawner))
        .set_scene_vehicles(std::move(vehicles));
}
