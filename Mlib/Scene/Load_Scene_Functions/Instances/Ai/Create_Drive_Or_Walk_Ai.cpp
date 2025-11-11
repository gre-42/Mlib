#include "Create_Drive_Or_Walk_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Vehicle_Ai/Drive_Or_Walk_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(waypoint_reached_radius);
DECLARE_ARGUMENT(rest_radius);
DECLARE_ARGUMENT(lookahead_velocity);
DECLARE_ARGUMENT(takeoff_velocity);
DECLARE_ARGUMENT(takeoff_velocity_delta);
DECLARE_ARGUMENT(max_velocity);
DECLARE_ARGUMENT(max_delta_velocity_brake);
DECLARE_ARGUMENT(collision_avoidance_radius_brake);
DECLARE_ARGUMENT(collision_avoidance_radius_wait);
DECLARE_ARGUMENT(collision_avoidance_radius_correct);
DECLARE_ARGUMENT(collision_avoidance_intersect_angle);
DECLARE_ARGUMENT(collision_avoidance_step_aside_angle);
DECLARE_ARGUMENT(collision_avoidance_step_aside_distance);
}

const std::string CreateDriveOrWalkAi::key = "create_drive_or_walk_ai";

LoadSceneJsonUserFunction CreateDriveOrWalkAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDriveOrWalkAi(args.physics_scene()).execute(args);
};

CreateDriveOrWalkAi::CreateDriveOrWalkAi(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateDriveOrWalkAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto ai = std::make_unique<DriveOrWalkAi>(
        player,
        args.arguments.at<ScenePos>(KnownArgs::waypoint_reached_radius) * meters,
        args.arguments.at<float>(KnownArgs::rest_radius) * meters,
        args.arguments.at<float>(KnownArgs::lookahead_velocity) * kph,
        args.arguments.at<float>(KnownArgs::takeoff_velocity) * kph,
        args.arguments.at<float>(KnownArgs::takeoff_velocity_delta) * kph,
        args.arguments.at<float>(KnownArgs::max_velocity) * kph,
        args.arguments.at<float>(KnownArgs::max_delta_velocity_brake) * kph,
        args.arguments.at<ScenePos>(KnownArgs::collision_avoidance_radius_brake) * meters,
        args.arguments.at<ScenePos>(KnownArgs::collision_avoidance_radius_wait) * meters,
        args.arguments.at<ScenePos>(KnownArgs::collision_avoidance_radius_correct) * meters,
        std::cos(args.arguments.at<float>(KnownArgs::collision_avoidance_intersect_angle) * degrees),
        std::cos(args.arguments.at<float>(KnownArgs::collision_avoidance_step_aside_angle) * degrees),
        args.arguments.at<float>(KnownArgs::collision_avoidance_step_aside_distance) * meters);
    player->rigid_body()->add_autopilot({ *ai, CURRENT_SOURCE_LOCATION });
    global_object_pool.add(std::move(ai), CURRENT_SOURCE_LOCATION);
}
