#include "Create_Gun_Key_Binding.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

DECLARE_OPTION(KEY);
DECLARE_OPTION(MOUSE_BUTTON);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(NOT_KEY);
DECLARE_OPTION(NOT_MOUSE_BUTTON);
DECLARE_OPTION(NOT_GAMEPAD_BUTTON);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(SELECT_NEXT_OPPONENT);

LoadSceneInstanceFunction::UserFunction CreateGunKeyBinding::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun_key_binding"
        "\\s+node=([\\w+-.]+)"

        "(?:\\s+key=(\\w+))?"
        "(?:\\s+mouse_button=(\\w+))?"
        "(?:\\s+gamepad_button=(\\w+))?"
        "(?:\\s+joystick_digital_axis=(\\w+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"

        "(?:\\s+not_key=(\\w+))?"
        "(?:\\s+not_mouse_button=(\\w+))?"
        "(?:\\s+not_gamepad_button=(\\w+))?"
        "(?:\\s+not_joystick_digital_axis=(\\w+)"
        "\\s+not_joystick_digital_axis_sign=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateGunKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateGunKeyBinding::CreateGunKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGunKeyBinding::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    if (match[NOT_KEY].matched ||
        match[NOT_GAMEPAD_BUTTON].matched ||
        match[NOT_JOYSTICK_DIGITAL_AXIS].matched)
    
    key_bindings.add_gun_key_binding(GunKeyBinding{
        .base_combo = BaseKeyCombination{
            .key_bindings = {
                BaseKeyBinding{
                    .key = match[KEY].str(),
                    .mouse_button = match[MOUSE_BUTTON].str(),
                    .gamepad_button = match[GAMEPAD_BUTTON].str(),
                    .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
                    .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].str().empty()
                        ? 0
                        : safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())}},
            .not_key_binding = BaseKeyBinding{
                .key = match[NOT_KEY].str(),
                .mouse_button = match[NOT_MOUSE_BUTTON].str(),
                .gamepad_button = match[NOT_GAMEPAD_BUTTON].str(),
                .joystick_axis = match[NOT_JOYSTICK_DIGITAL_AXIS].str(),
                .joystick_axis_sign = match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].str().empty()
                    ? 0
                    : safe_stof(match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].str())}},
        .node = scene.get_node(match[1].str())});
}
