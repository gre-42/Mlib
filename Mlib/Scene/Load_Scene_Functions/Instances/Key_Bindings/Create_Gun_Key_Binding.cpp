#include "Create_Gun_Key_Binding.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(UNIQUE);
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

LoadSceneUserFunction CreateGunKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun_key_binding"
        "\\s+unique=(\\w+)"
        "\\s+id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"

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
#ifdef _MSC_VER
    THROW_OR_ABORT("Keyword not supported under the MSC compiler due to a compiler bug");
#else
    auto& node = scene.get_node(match[NODE].str());
    auto* player = match[PLAYER].matched
        ? &players.get_player(match[PLAYER].str())
        : nullptr;
    auto& kb = key_bindings.add_gun_key_binding(GunKeyBinding{
        .id=match[ID].str(),
        .role=match[ROLE].str(),
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
#endif
}
