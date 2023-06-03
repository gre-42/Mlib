#include "Player_Set_Node.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
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
DECLARE_ARGUMENT(regex);
}

const std::string PlayerSetNode::key = "spawner_set_nodes";

LoadSceneJsonUserFunction PlayerSetNode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetNode(args.renderable_scene()).execute(args);
};

PlayerSetNode::PlayerSetNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetNode::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto names = args.arguments.at<std::string>(KnownArgs::regex);
    std::list<std::unique_ptr<SceneVehicle>> vehicles;
    for (const auto& [name, node] : scene.get_nodes(Mlib::compile_regex(names))) {
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
