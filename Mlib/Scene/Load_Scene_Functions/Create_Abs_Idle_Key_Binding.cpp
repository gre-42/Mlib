#include "Create_Abs_Idle_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(KEY);
DECLARE_OPTION(MOUSE_BUTTON);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);

LoadSceneInstanceFunction::UserFunction CreateAbsIdleKeyBinding::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*camera_key_binding"
        "\\s+key=([\\w+-.]+)"
        "\\s+gamepad_button=([\\w+-.]*)"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateAbsIdleKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateAbsIdleKeyBinding::CreateAbsIdleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAbsIdleKeyBinding::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    key_bindings.add_absolute_movable_idle_binding(AbsoluteMovableIdleBinding{
        .node = scene.get_node(match[1].str()),
        .tires_z = {
            match[2].str().empty() ? 0.f : safe_stof(match[2].str()),
            match[3].str().empty() ? 0.f : safe_stof(match[3].str()),
            match[4].str().empty() ? 1.f : safe_stof(match[4].str())}});
}
