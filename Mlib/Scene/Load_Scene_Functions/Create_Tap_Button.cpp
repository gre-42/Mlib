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
DECLARE_ARGUMENT(x_axis);
DECLARE_ARGUMENT(y_axis);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(viewport_id);
}

const std::string CreateTapButton::key = "create_tap_button";

template <class T, class TOperation>
std::optional<decltype(TOperation()(T()))> otransform(const std::optional<T>& v, const TOperation& op) {
    if (v.has_value()) {
        return op(*v);
    }
    return std::nullopt;
}

LoadSceneJsonUserFunction CreateTapButton::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto viewport_id = args.arguments.at<uint32_t>(KnownArgs::viewport_id);
    std::scoped_lock lock{args.button_states.tap_buttons_.at(viewport_id).mutex};
    auto key = args.arguments.try_at<std::string>(KnownArgs::key);
    args.button_states.tap_buttons_.at(viewport_id).button_states.push_back(
        TapButtonState{
            .key = otransform(
                    args.arguments.try_at<std::string>(KnownArgs::key),
                    [](const auto& v){ return tap_buttons_map.get(v); }),
            .joystick_xaxis = args.arguments.try_at<int>(KnownArgs::x_axis),
            .joystick_yaxis = args.arguments.try_at<int>(KnownArgs::y_axis),
            .widget = std::make_unique<Widget>(
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
                args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)))
            });
};
