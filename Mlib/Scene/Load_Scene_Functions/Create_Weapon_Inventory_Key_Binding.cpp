#include "Create_Weapon_Inventory_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Inventory_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

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

DECLARE_OPTION(SCROLL_WHEEL_AXIS);
DECLARE_OPTION(SCROLL_WHEEL_SIGN_AND_SCALE);
DECLARE_OPTION(WEAPON_INCREMENT);

LoadSceneInstanceFunction::UserFunction CreateWeaponInventoryKeyBinding::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*weapon_inventory_key_binding"
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
        CreateWeaponInventoryKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWeaponInventoryKeyBinding::CreateWeaponInventoryKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponInventoryKeyBinding::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    try {
        scene.get_node(match[1].str())->get_node_modifier();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Node \"" + match[1].str() + "\": " + e.what());
    }
    key_bindings.add_weapon_inventory_key_binding(WeaponInventoryKeyBinding{
        .base_key = {
            .key = match[2].str(),
            .gamepad_button = match[3].str(),
            .joystick_axis = match[4].str(),
            .joystick_axis_sign = safe_stof(match[5].str())},
        .base_scroll_wheel_axis = {
            .axis = match[6].matched ? safe_stou(match[6].str()) : SIZE_MAX,
            .sign_and_scale = match[7].matched ? safe_stof(match[7].str()) : NAN,
        },
        .scroll_wheel_movement = std::make_shared<CursorMovement>(scroll_wheel_states),
        .node = scene.get_node(match[1].str()),
        .direction = safe_stoi(match[8].str())});
}
