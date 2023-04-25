#include "Create_Tap_Button.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(key);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
}

const std::string CreateTapButton::key = "create_tap_button";

LoadSceneJsonUserFunction CreateTapButton::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{args.button_states.tap_buttons_.mutex};
    if (!args.button_states.tap_buttons_.button_states.insert({
        tap_buttons_map.get(args.arguments.at<std::string>(KnownArgs::key)),
        TapButtonState{
            .widget = std::make_unique<Widget>(
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
            .pressed = false}}).second)
    {
        THROW_OR_ABORT("Tap key binding \"" + args.arguments.at<std::string>(KnownArgs::key) + "\" already exists");
    }
};
