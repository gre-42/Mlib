#include "Create_Vehicle_Follower_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Follower_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
    CreateVehicleFollowerAi(args.renderable_scene()).execute(args);
};

CreateVehicleFollowerAi::CreateVehicleFollowerAi(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVehicleFollowerAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& missile_vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<std::string>(KnownArgs::missile), DP_LOC));
    auto& target_vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<std::string>(KnownArgs::target), DP_LOC));
    global_object_pool.create<FollowerAi>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        DanglingBaseClassRef<RigidBodyVehicle>{ missile_vehicle, CURRENT_SOURCE_LOCATION },
        DanglingBaseClassRef<RigidBodyVehicle>{ target_vehicle, CURRENT_SOURCE_LOCATION });
}
