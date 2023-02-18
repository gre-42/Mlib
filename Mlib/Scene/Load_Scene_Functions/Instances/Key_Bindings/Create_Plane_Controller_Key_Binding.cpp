#include "Create_Plane_Controller_Key_Binding.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);
DECLARE_OPTION(TAP_BUTTON);

DECLARE_OPTION(NOT_KEY);
DECLARE_OPTION(NOT_GAMEPAD_BUTTON);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS_SIGN);
DECLARE_OPTION(NOT_TAP_BUTTON);

DECLARE_OPTION(JOYSTICK_ANALOG_AXIS);
DECLARE_OPTION(JOYSTICK_ANALOG_AXIS_SIGN_AND_SCALE);

DECLARE_OPTION(TURBINE_POWER);
DECLARE_OPTION(BRAKE);
DECLARE_OPTION(PITCH);
DECLARE_OPTION(YAW);
DECLARE_OPTION(ROLL);

LoadSceneUserFunction CreatePlaneControllerKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*plane_controller_key_binding"
        "(?:\\s+player=([\\w+-.]+))?"
        "\\s+node=([\\w+-.]+)"

        "(?:\\s+key=([\\w+-.]+))?"
        "(?:\\s+gamepad_button=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+tap_button=([\\w+-.]+))?"

        "(?:\\s+not_key=([\\w+-.]+))?"
        "(?:\\s+not_gamepad_button=([\\w+-.]+))?"
        "(?:\\s+not_joystick_digital_axis=([\\w+-.]+)"
        "\\s+not_joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+not_tap_button=([\\w+-.]+))?"

        "(?:\\s+joystick_analog_axis=([\\w+-.]+)?"
        "\\s+joystick_analog_axis_sign_and_scale=([\\w+-.]+)?)?"

        "(?:\\s+turbine_power=([\\w+-.]+))?"
        "(?:\\s+brake=([\\w+-.]+))?"
        "(?:\\s+pitch=([ \\w+-.]+))?"
        "(?:\\s+yaw=([ \\w+-.]+))?"
        "(?:\\s+roll=([ \\w+-.]+))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreatePlaneControllerKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreatePlaneControllerKeyBinding::CreatePlaneControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto& kb = key_bindings.add_plane_controller_key_binding(PlaneControllerKeyBinding{
        .base_combo = {
            .key_bindings = {
                BaseKeyBinding{
                    .key = match[KEY].str(),
                    .gamepad_button = match[GAMEPAD_BUTTON].str(),
                    .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
                    .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                        ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                        : 0}},
            .not_key_binding = BaseKeyBinding{
                .key = match[NOT_KEY].str(),
                .gamepad_button = match[NOT_GAMEPAD_BUTTON].str(),
                .joystick_axis = match[NOT_JOYSTICK_DIGITAL_AXIS].str(),
                .joystick_axis_sign = match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].matched
                    ? safe_stof(match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].str())
                    : 0,
                .tap_button = match[NOT_TAP_BUTTON].str()}},
        .base_gamepad_analog_axis = BaseGamepadAnalogAxisBinding{
            .axis = match[JOYSTICK_ANALOG_AXIS].str(),
            .sign_and_scale = match[JOYSTICK_ANALOG_AXIS_SIGN_AND_SCALE].matched
                ? safe_stof(match[JOYSTICK_ANALOG_AXIS_SIGN_AND_SCALE].str())
                : NAN},
        .node = &node,
        .turbine_power = match[TURBINE_POWER].matched
            ? safe_stof(match[TURBINE_POWER].str()) * W
            : std::optional<float>(),
        .brake = match[BRAKE].matched
            ? safe_stof(match[BRAKE].str()) * degrees
            : std::optional<float>(),
        .pitch = match[PITCH].matched
            ? safe_stof(match[PITCH].str()) * degrees
            : std::optional<float>(),
        .yaw = match[YAW].matched
            ? safe_stof(match[YAW].str()) * degrees
            : std::optional<float>(),
        .roll = match[ROLL].matched
            ? safe_stof(match[ROLL].str()) * degrees
            : std::optional<float>(),});
    if (match[PLAYER].matched) {
        players.get_player(match[PLAYER].str())
        .append_delete_externals(
            &node,
            [&kbs=key_bindings, &kb](){
                kbs.delete_plane_controller_key_binding(kb);
            }
        );
    }
}
