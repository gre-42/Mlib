#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
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
DECLARE_OPTION(SELECT_NEXT_OPPONENT);

LoadSceneInstanceFunction::UserFunction CreateDriverKeyBinding::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(\\s+select_next_opponent)?$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateDriverKeyBinding(renderable_scene()).execute(
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

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    SceneNode* node = scene.get_node(match[NODE].str());
    bool select_next_opponent = match[SELECT_NEXT_OPPONENT].matched;
    key_bindings.add_player_key_binding(PlayerKeyBinding{
        .base_key = {
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0 },
        .node = node,
        .select_next_opponent = select_next_opponent});
}
