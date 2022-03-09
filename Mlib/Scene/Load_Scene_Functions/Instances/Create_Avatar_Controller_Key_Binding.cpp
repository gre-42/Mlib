#include "Create_Avatar_Controller_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);
DECLARE_OPTION(CURSOR_AXIS);
DECLARE_OPTION(CURSOR_SIGN_AND_SCALE);
DECLARE_OPTION(SURFACE_POWER);
DECLARE_OPTION(YAW);
DECLARE_OPTION(PITCH);
DECLARE_OPTION(ANGULAR_VELOCITY_PRESS);
DECLARE_OPTION(ANGULAR_VELOCITY_REPEAT);
DECLARE_OPTION(SPEED_CURSOR);
DECLARE_OPTION(TIRE_Z_0);
DECLARE_OPTION(TIRE_Z_1);
DECLARE_OPTION(TIRE_Z_2);

LoadSceneUserFunction CreateAvatarControllerKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*avatar_controller_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*))?"
        "(?:\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+cursor_axis=(0|1)?"
        "\\s+cursor_sign_and_scale=([\\w+-.]+))?"
        "(?:\\s+surface_power=([\\w+-.]+))?"
        "(?:\\s+yaw())?"
        "(?:\\s+pitch())?"
        "(?:\\s+angular_velocity_press=([\\w+-.]+))?"
        "(?:\\s+angular_velocity_repeat=([\\w+-.]+))?"
        "(?:\\s+speed_cursor=([\\w+-.]+))?"
        "(?:\\s+tires_z=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateAvatarControllerKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateAvatarControllerKeyBinding::CreateAvatarControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_avatar_controller_key_binding(AvatarControllerKeyBinding{
        .base_key = {
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? 0
                : safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())},
        .base_cursor_axis = {
            .axis = match[CURSOR_AXIS].matched
                ? safe_stou(match[CURSOR_AXIS].str())
                : SIZE_MAX,
            .sign_and_scale = match[CURSOR_SIGN_AND_SCALE].matched
                ? safe_stof(match[CURSOR_SIGN_AND_SCALE].str())
                : NAN,
        },
        .cursor_movement = match[CURSOR_AXIS].matched
            ? std::make_shared<CursorMovement>(cursor_states)
            : nullptr,
        .node = &scene.get_node(match[NODE].str()),
        .surface_power = match[SURFACE_POWER].matched
            ? safe_stof(match[SURFACE_POWER].str())
            : std::optional<float>(),
        .yaw = match[YAW].matched,
        .pitch = match[PITCH].matched,
        .angular_velocity_press = match[ANGULAR_VELOCITY_PRESS].matched
            ? safe_stof(match[ANGULAR_VELOCITY_PRESS].str())
            : std::optional<float>(),
        .angular_velocity_repeat = match[ANGULAR_VELOCITY_REPEAT].matched
            ? safe_stof(match[ANGULAR_VELOCITY_REPEAT].str())
            : std::optional<float>(),
        .speed_cursor = match[SPEED_CURSOR].matched
            ? safe_stof(match[SPEED_CURSOR].str())
            : std::optional<float>(),
        .tires_z = match[TIRE_Z_0].matched
            ? FixedArray<float, 3>{
                safe_stof(match[TIRE_Z_0].str()),
                safe_stof(match[TIRE_Z_1].str()),
                safe_stof(match[TIRE_Z_2].str())}
            : std::optional<FixedArray<float, 3>>()});
}
