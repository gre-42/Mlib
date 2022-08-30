#include "Create_Weapon_Cycle_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(SCROLL_WHEEL_AXIS);
DECLARE_OPTION(SCROLL_WHEEL_SIGN_AND_SCALE);
DECLARE_OPTION(WEAPON_INCREMENT);

LoadSceneUserFunction CreateWeaponCycleKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*weapon_cycle_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+)"
        "(?:\\s+scroll_wheel_axis=(0|1))?"
        "(?:\\s+scroll_wheel_sign_and_scale=([\\w+-.]+))?"
        "\\s+weapon_increment=([\\d-]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateWeaponCycleKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWeaponCycleKeyBinding::CreateWeaponCycleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycleKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_weapon_inventory_key_binding(WeaponCycleKeyBinding{
        .base_key = {
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())},
        .base_scroll_wheel_axis = {
            .axis = match[SCROLL_WHEEL_AXIS].matched ? safe_stou(match[SCROLL_WHEEL_AXIS].str()) : SIZE_MAX,
            .sign_and_scale = match[SCROLL_WHEEL_SIGN_AND_SCALE].matched ? safe_stof(match[SCROLL_WHEEL_SIGN_AND_SCALE].str()) : NAN,
        },
        .scroll_wheel_movement = std::make_shared<CursorMovement>(scroll_wheel_states),
        .node = &scene.get_node(match[NODE].str()),
        .direction = safe_stoi(match[WEAPON_INCREMENT].str())});
}
