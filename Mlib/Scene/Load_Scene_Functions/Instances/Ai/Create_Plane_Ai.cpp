#include "Create_Plane_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Vehicle_Ai/Plane_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(ai_name);
DECLARE_ARGUMENT(taxi);
DECLARE_ARGUMENT(fly);
}

const std::string CreatePlaneAi::key = "create_plane_ai";

LoadSceneJsonUserFunction CreatePlaneAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlaneAi(args.renderable_scene()).execute(args);
};

CreatePlaneAi::CreatePlaneAi(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), DP_LOC);
    auto& vehicle = player->rigid_body();
    auto taxi_ai = vehicle.get_autopilot(args.arguments.at<std::string>(KnownArgs::taxi));
    auto fly_ai = vehicle.get_autopilot(args.arguments.at<std::string>(KnownArgs::fly));
    auto plane_ai = std::make_unique<PlaneAi>(
        DanglingBaseClassRef<RigidBodyVehicle>{ vehicle, CURRENT_SOURCE_LOCATION },
        taxi_ai,
        fly_ai);
    vehicle.set_autopilot(
        args.arguments.at<std::string>(KnownArgs::ai_name),
        { *plane_ai, CURRENT_SOURCE_LOCATION });
    global_object_pool.add(std::move(plane_ai), CURRENT_SOURCE_LOCATION);
}
