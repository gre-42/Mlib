#include "Player_Set_Vehicle_Control_Parameters.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(surface_power_forward);
DECLARE_ARGUMENT(surface_power_backward);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(tire_angle_pid);
}

namespace TAP {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(alpha);
}

const std::string PlayerSetVehicleControlParameters::key = "player_set_vehicle_control_parameters";

LoadSceneJsonUserFunction PlayerSetVehicleControlParameters::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (args.arguments.contains(KnownArgs::tire_angle_pid)) {
        args.arguments.child(KnownArgs::tire_angle_pid).validate(TAP::options);
    }
    PlayerSetVehicleControlParameters(args.renderable_scene()).execute(args);
};

PlayerSetVehicleControlParameters::PlayerSetVehicleControlParameters(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetVehicleControlParameters::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references
        .get_replacement_parameters("vehicles")
        .at(asset_id)
        .rp;
    auto& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    player.vehicle_movement.set_control_parameters(
        args.arguments.at<float>(KnownArgs::surface_power_forward) * W,
        args.arguments.at<float>(KnownArgs::surface_power_backward) * W);
    auto max_tire_angle = vars.globals.at("MAX_TIRE_ANGLE");
    if (max_tire_angle.type() != nlohmann::detail::value_t::null) {
        player.car_movement.set_max_tire_angle(
            max_tire_angle.get<float>() * degrees);
    }
    if (args.arguments.contains(KnownArgs::tire_angle_pid)) {
        auto c = args.arguments.child(KnownArgs::tire_angle_pid);
        auto pid = c.at<FixedArray<float, 3>>(TAP::pid);
        player.car_movement.set_tire_angle_pid(
            PidController<float, float>{
                pid(0),
                pid(1),
                pid(2),
                c.at<float>(TAP::alpha)});
    }
}
