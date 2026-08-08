#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/OpenGL/Ui/Button_States.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(key);
DECLARE_ARGUMENT(x_axis);
DECLARE_ARGUMENT(y_axis);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
}

template <class T, class TOperation>
static std::optional<decltype(TOperation()(T()))> otransform(const std::optional<T>& v, const TOperation& op) {
    if (v.has_value()) {
        return op(*v);
    }
    return std::nullopt;
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_tap_button",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto local_user_id = args.arguments.at<NUserCountType>(KnownArgs::local_user_id);
                std::scoped_lock lock{args.button_states.tap_buttons_mutex_};
                auto key = args.arguments.try_at<std::string>(KnownArgs::key);
                args.button_states.tap_buttons_[local_user_id].button_states.push_back(
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
            });
    }
} obj;

}
