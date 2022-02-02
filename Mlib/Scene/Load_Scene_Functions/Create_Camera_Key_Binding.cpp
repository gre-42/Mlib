#include "Create_Camera_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
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

LoadSceneInstanceFunction::UserFunction CreateCameraKeyBinding::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*camera_key_binding"
        "(?:\\s+key=([\\w+-.]+))?"
        "(?:\\s+mouse_button=(\\w+))?"
        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateCameraKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateCameraKeyBinding::CreateCameraKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCameraKeyBinding::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    key_bindings.add_camera_key_binding(CameraKeyBinding{
        .base = {
            .key = match[KEY].str(),
            .mouse_button = match[MOUSE_BUTTON].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0.f}});
}
