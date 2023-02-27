#include "Create_Tap_Button.hpp"
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
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
    std::scoped_lock lock{args.button_states.tap_buttons_.mutex};
    if (!args.button_states.tap_buttons_.button_states.insert({
        tap_buttons_map.get(match[KEY].str()),
        TapButtonState{
            .widget = std::make_unique<Widget>(
                args.layout_constraints.get_pixels(match[LEFT].str()),
                args.layout_constraints.get_pixels(match[RIGHT].str()),
                args.layout_constraints.get_pixels(match[BOTTOM].str()),
                args.layout_constraints.get_pixels(match[TOP].str())),
            .pressed = false}}).second)
    {
        THROW_OR_ABORT("Tap key binding \"" + match[KEY].str() + "\" already exists");
    }
}
