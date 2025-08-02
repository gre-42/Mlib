#include "Input_State.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Input_State_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(update_interval_ms);
DECLARE_ARGUMENT(submenus);
}

InputState::InputState(RenderableScene& renderable_scene)
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void InputState::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto id = args.arguments.at<std::string>(KnownArgs::id);
    auto focus_filter = FocusFilter{
        .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        .submenu_ids = args.arguments.at<std::set<std::string>>(KnownArgs::submenus, { id }) };
    ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = args.arguments.at<std::string>(KnownArgs::title)
        },
        focus_filter,
        0);
    auto& input_state_logic = object_pool.create<InputStateLogic>(
        CURRENT_SOURCE_LOCATION,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        std::move(focus_filter),
        std::chrono::milliseconds{ args.arguments.at<uint32_t>(KnownArgs::update_interval_ms) },
        args.button_states);
    render_logics.append(
        { input_state_logic, CURRENT_SOURCE_LOCATION },
        1,                          // z_order
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "input_state",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                InputState(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
