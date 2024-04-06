#include "Create_Missile_Ai.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Missile_Controller.hpp>
#include <Mlib/Players/Advance_Times/Vehicle_Ai_Advance_Time.hpp>
#include <Mlib/Players/Vehicle_Ai/Missile_Ai.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(missile);
DECLARE_ARGUMENT(target);
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(destination_reached_radius);
}

namespace PidArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(alpha);
}

const std::string CreateMissileAi::key = "create_missile_ai";

LoadSceneJsonUserFunction CreateMissileAi::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateMissileAi(args.renderable_scene()).execute(args);
};

CreateMissileAi::CreateMissileAi(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateMissileAi::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto jpid = args.arguments.child(KnownArgs::pid);
    jpid.validate(PidArgs::options);
    auto pid = jpid.at<FixedArray<float, 3>>(PidArgs::pid);
    auto pid_alpha = jpid.at<float>(PidArgs::alpha);
    PidController<FixedArray<float, 3>, float> pid_controller{
        pid(0),
        pid(1) * (physics_engine.config().dt / s),
        pid(2) / (physics_engine.config().dt / s),
        std::pow(pid_alpha, physics_engine.config().dt / (1.f / 60.f * s)) };
    auto& missile_vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<std::string>(KnownArgs::missile), DP_LOC));
    auto& target_vehicle = get_rigid_body_vehicle(scene.get_node(args.arguments.at<std::string>(KnownArgs::target), DP_LOC));
    auto missile_ai = std::make_unique<MissileAi>(
        pid_controller,
        missile_vehicle.missile_controller(),
        missile_vehicle.rbp_,
        target_vehicle.rbp_,
        args.arguments.at<float>(KnownArgs::destination_reached_radius));
    new VehicleAiAdvanceTime(
        physics_engine.advance_times_,
        std::move(missile_ai),
        { missile_vehicle, CURRENT_SOURCE_LOCATION },
        { target_vehicle, CURRENT_SOURCE_LOCATION });
}
