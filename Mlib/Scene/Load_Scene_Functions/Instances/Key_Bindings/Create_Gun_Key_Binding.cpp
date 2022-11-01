#include "Create_Gun_Key_Binding.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
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

LoadSceneUserFunction CreateGunKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun_key_binding"
        "(?:\\s+player=([\\w+-.]+))?"
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
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto* player = match[PLAYER].matched
        ? &players.get_player(match[PLAYER].str())
        : nullptr;
    auto& kb = key_bindings.add_gun_key_binding(GunKeyBinding{
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
                .joystick_axis_sign = match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].matched
                    ? safe_stof(match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].str())
                    : 0.f}},
        .node = &node,
        .player = player});
    if (player != nullptr) {
        player->append_delete_externals(
            &node,
            [&kbs=key_bindings, &kb](){
                kbs.delete_gun_key_binding(kb);
            }
        );
    }
}
