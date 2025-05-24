#include "Create_Destination_Reached_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Players/Vehicle_Ai/Destination_Reached_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(destination_reached_radius);
DECLARE_ARGUMENT(control_source);
}


const std::string CreateDestinationReachedAi::key = "create_destination_reached_ai";

LoadSceneJsonUserFunction CreateDestinationReachedAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDestinationReachedAi(args.physics_scene()).execute(args);
};

CreateDestinationReachedAi::CreateDestinationReachedAi(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateDestinationReachedAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::vehicle), DP_LOC));
    vehicle.add_autopilot(
        {
            global_object_pool.create<DestinationReachedAi>(
                CURRENT_SOURCE_LOCATION,
                vehicle,
                control_source_from_string(args.arguments.at<std::string>(KnownArgs::control_source)),
                args.arguments.at<float>(KnownArgs::destination_reached_radius)),
            CURRENT_SOURCE_LOCATION
        });
}
