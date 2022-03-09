#include "Create_Rel_Key_Binding.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

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
DECLARE_OPTION(CURSOR_AXIS);
DECLARE_OPTION(CURSOR_SIGN_AND_SCALE);

DECLARE_OPTION(NOT_KEY);
DECLARE_OPTION(NOT_GAMEPAD_BUTTON);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(ROTATION_AXIS_X);
DECLARE_OPTION(ROTATION_AXIS_Y);
DECLARE_OPTION(ROTATION_AXIS_Z);
DECLARE_OPTION(ANGULAR_VELOCITY_PRESS);
DECLARE_OPTION(ANGULAR_VELOCITY_REPEAT);
DECLARE_OPTION(SPEED_CURSOR);

LoadSceneUserFunction CreateRelKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rel_key_binding"
        "(?:\\s+player=([\\w+-.]+))?"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"

        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+cursor_axis=(0|1))?"
        "(?:\\s+cursor_sign_and_scale=([\\w+-.]+))?"

        "(?:\\s+not_key=([\\w+-.]+))?"
        "(?:\\s+not_gamepad_button=([\\w+-.]+))?"
        "(?:\\s+not_joystick_digital_axis=([\\w+-.]+)"
        "\\s+not_joystick_digital_axis_sign=([\\w+-.]+))?"

        "\\s+rotation_axis=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+angular_velocity_press=([\\w+-.]+)"
        "\\s+angular_velocity_repeat=([\\w+-.]+)"
        "\\s+speed_cursor=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRelKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRelKeyBinding::CreateRelKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    try {
        scene.get_node(match[NODE].str()).get_relative_movable();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Node \"" + match[NODE].str() + "\": " + e.what());
    }
    key_bindings.add_relative_movable_key_binding(RelativeMovableKeyBinding{
        .base_combo = {
            .key_bindings = {BaseKeyBinding{
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
                    : 0}},
        .base_cursor_axis = {
            .axis = match[CURSOR_AXIS].matched ? safe_stou(match[CURSOR_AXIS].str()) : SIZE_MAX,
            .sign_and_scale = match[CURSOR_SIGN_AND_SCALE].matched ? safe_stof(match[CURSOR_SIGN_AND_SCALE].str()) : NAN,
        },
        .cursor_movement = std::make_shared<CursorMovement>(cursor_states),
        .node = &scene.get_node(match[NODE].str()),
        .rotation_axis = {
            safe_stof(match[ROTATION_AXIS_X].str()),
            safe_stof(match[ROTATION_AXIS_Y].str()),
            safe_stof(match[ROTATION_AXIS_Z].str())},
        .angular_velocity_press = safe_stof(match[ANGULAR_VELOCITY_PRESS].str()),
        .angular_velocity_repeat = safe_stof(match[ANGULAR_VELOCITY_REPEAT].str()),
        .speed_cursor = safe_stof(match[SPEED_CURSOR].str())});
}
