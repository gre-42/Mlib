#include "Player_Set_Vehicle_Control_Parameters.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player_name);
DECLARE_ARGUMENT(surface_power_forward);
DECLARE_ARGUMENT(surface_power_backward);
DECLARE_ARGUMENT(max_tire_angle);
DECLARE_ARGUMENT(tire_angle_pid);

namespace TAP {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(alpha);
}

const std::string PlayerSetVehicleControlParameters::key = "player_set_vehicle_control_parameters";

LoadSceneUserFunction PlayerSetVehicleControlParameters::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(options);
    if (json_macro_arguments.contains_json(tire_angle_pid)) {
        JsonMacroArguments tap{json_macro_arguments.at(tire_angle_pid)};
        tap.validate(TAP::options);
        json_macro_arguments.insert_child(tire_angle_pid, std::move(tap));
    }
    PlayerSetVehicleControlParameters(args.renderable_scene()).execute(json_macro_arguments, args);
};

PlayerSetVehicleControlParameters::PlayerSetVehicleControlParameters(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetVehicleControlParameters::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    auto& player = players.get_player(json_macro_arguments.at<std::string>(player_name));
    player.vehicle_movement.set_control_parameters(
        json_macro_arguments.at<float>(surface_power_forward) * W,
        json_macro_arguments.at<float>(surface_power_backward) * W);
    if (json_macro_arguments.contains_json(max_tire_angle)) {
        player.car_movement.set_max_tire_angle(
            json_macro_arguments.at<float>(max_tire_angle) * degrees);
    }
    if (json_macro_arguments.contains_child(tire_angle_pid)) {
        auto& c = json_macro_arguments.child(tire_angle_pid);
        auto pid = c.at<FixedArray<float, 3>>(TAP::pid);
        player.car_movement.set_tire_angle_pid(
            PidController<float, float>{
                pid(0),
                pid(1),
                pid(2),
                c.at<float>(TAP::alpha)});
    }
}
