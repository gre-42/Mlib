#include "Spawner_Set_Nodes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
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
    SpawnerSetNodes(args.renderable_scene()).execute(args);
};

SpawnerSetNodes::SpawnerSetNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SpawnerSetNodes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references
        .get_replacement_parameters("vehicles")
        .at(asset_id);
    auto prefixes = vars.globals.at<std::vector<std::string>>("NODE_PREFIXES");
    auto suffix = args.arguments.at<std::string>(KnownArgs::suffix);
    std::list<std::unique_ptr<SceneVehicle>> vehicles;
    for (const auto& prefix : prefixes) {
        auto name = prefix + suffix;
        auto& node = scene.get_node(name);
        auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Follower movable is not a rigid body");
        }
        vehicles.push_back(std::make_unique<SceneVehicle>(delete_node_mutex, name, node, *rb));
    }
    vehicle_spawners
        .get(args.arguments.at<std::string>(KnownArgs::spawner))
        .set_scene_vehicles(std::move(vehicles));
}