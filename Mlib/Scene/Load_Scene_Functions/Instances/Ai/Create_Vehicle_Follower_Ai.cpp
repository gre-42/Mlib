#include "Create_Vehicle_Follower_Ai.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Follower_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(missile);
DECLARE_ARGUMENT(target);
}

const std::string CreateVehicleFollowerAi::key = "create_vehicle_follower_ai";

LoadSceneJsonUserFunction CreateVehicleFollowerAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateVehicleFollowerAi(args.physics_scene()).execute(args);
};

CreateVehicleFollowerAi::CreateVehicleFollowerAi(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateVehicleFollowerAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto missile_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::missile), CURRENT_SOURCE_LOCATION);
    auto target_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::target), CURRENT_SOURCE_LOCATION);
    auto missile_vehicle = get_rigid_body_vehicle(missile_node.get(), CURRENT_SOURCE_LOCATION);
    auto target_vehicle = get_rigid_body_vehicle(target_node.get(), CURRENT_SOURCE_LOCATION);
    global_object_pool.create<FollowerAi>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        missile_vehicle,
        target_vehicle);
}
