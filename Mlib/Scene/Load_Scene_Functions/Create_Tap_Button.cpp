#include "Create_Tap_Button.hpp"
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Ui/Tap_Buttons_States.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(KEY);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);

LoadSceneUserFunction CreateTapButton::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_tap_button"
        "\\s+key=(\\w+)"
        "\\s+left=([\\w+-.]+)"
        "\\s+right=([\\w+-.]+)"
        "\\s+bottom=([\\w+-.]+)"
        "\\s+top=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateTapButton::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard lock{args.tap_buttons_states.mutex};
    if (!args.tap_buttons_states.button_states.insert({
        tap_buttons_map.get(match[KEY].str()),
        TapButtonState{
            .left = safe_stof(match[LEFT].str()),
            .right = safe_stof(match[RIGHT].str()),
            .bottom = safe_stof(match[BOTTOM].str()),
            .top = safe_stof(match[TOP].str()),
            .pressed = false}}).second)
    {
        THROW_OR_ABORT("Tap key binding \"" + match[KEY].str() + "\" already exists");
    }
}
