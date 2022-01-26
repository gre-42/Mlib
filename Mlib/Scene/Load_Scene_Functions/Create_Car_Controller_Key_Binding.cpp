#include "Create_Car_Controller_Key_Binding.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
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
DECLARE_OPTION(SURFACE_POWER);
DECLARE_OPTION(TIRE_ANGLE_VELOCITIES);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(ASCEND_VELOCITY);

LoadSceneInstanceFunction::UserFunction CreateCarControllerKeyBinding::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*car_controller_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]*))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]*)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+surface_power=([\\w+-.]+))?"
        "(?:\\s+tire_angle_velocities=([ \\w+-.]+)"
        "\\s+tire_angles=([ \\w+-.]+))?"
        "(?:\\s+ascend_velocity=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateCarControllerKeyBinding(renderable_scene()).execute(
            match,
            fpath,
            macro_line_executor,
            local_substitutions,
            rsc);
        return true;
    } else {
        return false;
    }
};

CreateCarControllerKeyBinding::CreateCarControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCarControllerKeyBinding::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    key_bindings.add_car_controller_key_binding(CarControllerKeyBinding{
        .base_key = {
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0},
        .node = scene.get_node(match[NODE].str()),
        .surface_power = match[SURFACE_POWER].matched
            ? safe_stof(match[SURFACE_POWER].str())
            : std::optional<float>(),
        .tire_angle_interp = match[TIRE_ANGLE_VELOCITIES].matched
            ? Interp<float>{
                string_to_vector(match[TIRE_ANGLE_VELOCITIES].str(), safe_stof),
                string_to_vector(match[TIRE_ANGLES].str(), safe_stof),
                OutOfRangeBehavior::CLAMP}
            : std::optional<Interp<float>>(),
        .ascend_velocity = match[ASCEND_VELOCITY].matched
            ? safe_stof(match[ASCEND_VELOCITY].str())
            : std::optional<float>()});
}
