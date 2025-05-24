#include "Create_Missile_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Missile_Controller.hpp>
#include <Mlib/Players/Vehicle_Ai/Flying_Missile_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(missile);
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(dy);
DECLARE_ARGUMENT(eta_max);
DECLARE_ARGUMENT(waypoint_reached_radius);
DECLARE_ARGUMENT(resting_position_reached_radius);
DECLARE_ARGUMENT(maximum_velocity);
}

namespace PidArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(alpha);
}

namespace DyArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(dy);
}

const std::string CreateMissileAi::key = "create_missile_ai";

static inline float parse_kph(float v) {
    return v * kph;
}

LoadSceneJsonUserFunction CreateMissileAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateMissileAi(args.physics_scene()).execute(args);
};

CreateMissileAi::CreateMissileAi(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateMissileAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto jpid = args.arguments.child(KnownArgs::pid);
    jpid.validate(PidArgs::options);
    auto pid = jpid.at<UFixedArray<float, 3>>(PidArgs::pid);
    auto pid_alpha = jpid.at<float>(PidArgs::alpha);
    auto pid_controller = PidController<FixedArray<float, 3>, float>{
        pid(0) / meters,
        pid(1) / meters,
        pid(2) / meters,
        pid_alpha}.changed_time_step(1.f / 60.f * seconds, physics_engine.config().dt);
    auto jdy = args.arguments.child(KnownArgs::dy);
    jdy.validate(DyArgs::options);
    Interp<float, float> dy{
        jdy.at_vector<float>(DyArgs::velocity, parse_kph),
        jdy.at<std::vector<float>>(DyArgs::dy),
        OutOfRangeBehavior::CLAMP };
    auto& missile_vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::missile), DP_LOC));
    missile_vehicle.add_autopilot(
        {
            global_object_pool.create<FlyingMissileAi>(
                CURRENT_SOURCE_LOCATION,
                missile_vehicle,
                pid_controller,
                std::move(dy),
                args.arguments.at<float>(KnownArgs::eta_max) * seconds,
                missile_vehicle.missile_controller(),
                args.arguments.at<float>(KnownArgs::waypoint_reached_radius) * meters,
                args.arguments.at<float>(KnownArgs::resting_position_reached_radius) * meters,
                args.arguments.at<float>(KnownArgs::maximum_velocity) * kph),
            CURRENT_SOURCE_LOCATION
        });
}
