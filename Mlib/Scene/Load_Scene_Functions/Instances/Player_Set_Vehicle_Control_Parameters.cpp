#include "Player_Set_Vehicle_Control_Parameters.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(SURFACE_POWER_FORWARD);
DECLARE_OPTION(SURFACE_POWER_BACKWARD);
DECLARE_OPTION(MAX_TIRE_ANGLE);
DECLARE_OPTION(TIRE_ANGLE_PID_P);
DECLARE_OPTION(TIRE_ANGLE_PID_I);
DECLARE_OPTION(TIRE_ANGLE_PID_D);
DECLARE_OPTION(TIRE_ANGLE_PID_ALPHA);

LoadSceneUserFunction PlayerSetVehicleControlParameters::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_set_vehicle_control_parameters"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+surface_power_forward=([\\w+-.]+)"
        "\\s+surface_power_backward=([\\w+-.]+)"
        "\\s+max_tire_angle=([\\w+-.]*)"
        "\\s+tire_angle_pid=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetVehicleControlParameters(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetVehicleControlParameters::PlayerSetVehicleControlParameters(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetVehicleControlParameters::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.get_player(match[PLAYER_NAME].str()).set_vehicle_control_parameters(
        safe_stof(match[SURFACE_POWER_FORWARD].str()) * W,
        safe_stof(match[SURFACE_POWER_BACKWARD].str()) * W,
        safe_stof(match[MAX_TIRE_ANGLE].str()) * degrees,
        PidController<float, float>{
            safe_stof(match[TIRE_ANGLE_PID_P].str()),
            safe_stof(match[TIRE_ANGLE_PID_I].str()),
            safe_stof(match[TIRE_ANGLE_PID_D].str()),
            safe_stof(match[TIRE_ANGLE_PID_ALPHA].str())});
}
