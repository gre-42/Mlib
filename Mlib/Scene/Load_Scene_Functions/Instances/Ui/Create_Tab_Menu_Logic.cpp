#include "Create_Tab_Menu_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Constraint_Window.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene/Render_Logics/Tab_Menu_Logic.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(selection_marker);
DECLARE_ARGUMENT(style);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(reference_widget);
DECLARE_ARGUMENT(icon_widget);
DECLARE_ARGUMENT(title_widget);
DECLARE_ARGUMENT(widget);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(deflt);
DECLARE_ARGUMENT(on_execute);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(focus_mask);
}

CreateTabMenuLogic::CreateTabMenuLogic(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateTabMenuLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string id = args.arguments.at<std::string>(KnownArgs::id);
    auto reference_widget = args.layout_constraints.get_widget(
        args.arguments.at<ConstraintWindow>(KnownArgs::reference_widget));
    auto icon_widget = args.arguments.contains(KnownArgs::icon_widget)
        ? args.layout_constraints.get_widget(args.arguments.at<ConstraintWindow>(KnownArgs::icon_widget))
        : nullptr;
    auto title_widget = args.arguments.contains(KnownArgs::title_widget)
        ? args.layout_constraints.get_widget(args.arguments.at<ConstraintWindow>(KnownArgs::title_widget))
        : nullptr;
    auto widget = args.layout_constraints.get_widget(
        args.arguments.at<ConstraintWindow>(KnownArgs::widget));
    size_t deflt = args.arguments.at<size_t>(KnownArgs::deflt);
    std::function<void()> on_execute;
    if (auto ooe = args.arguments.try_at(KnownArgs::on_execute); ooe.has_value()) {
        on_execute = [macro_line_executor = args.macro_line_executor, oe=*ooe]() {
            macro_line_executor(oe, nullptr, nullptr);
            // This results in a deadlock because both "delete_node_mutex" and "delete_rigid_body_mutex" are acquired.
            // std::scoped_lock rb_lock{ delete_rigid_body_mutex };
            // macro_line_executor(reload_transient_objects, nullptr);
        };
    }
    // If the selection_ids array is not yet initialized, apply the default value.
    ui_focus.all_selection_ids.try_emplace(id, deflt);
    auto& tab_menu_logic = object_pool.create<TabMenuLogic>(
        CURRENT_SOURCE_LOCATION,
        std::move(id),
        focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        args.confirm_button_press,
        args.gallery,
        list_view_style_from_string(args.arguments.at<std::string>(KnownArgs::style)),
        args.arguments.at<std::string>(KnownArgs::selection_marker),
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        std::move(reference_widget),
        std::move(icon_widget),
        std::move(title_widget),
        std::move(widget),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        args.external_json_macro_arguments,
        args.asset_references,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),
        ui_focus,
        args.num_renderings,
        args.button_states,
        on_execute);
    render_logics.append(
        { tab_menu_logic, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order, 0),
        CURRENT_SOURCE_LOCATION);
}


namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "tab_menu",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateTabMenuLogic(args.renderable_scene()).execute(args);            
            });
    }
} obj;

}
