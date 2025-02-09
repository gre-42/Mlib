#include "Create_Tab_Menu_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene/Render_Logics/Tab_Menu_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(selection_marker);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(icon_left);
DECLARE_ARGUMENT(icon_right);
DECLARE_ARGUMENT(icon_bottom);
DECLARE_ARGUMENT(icon_top);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(deflt);
DECLARE_ARGUMENT(on_execute);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(focus_mask);
}

const std::string CreateTabMenuLogic::key = "tab_menu";

LoadSceneJsonUserFunction CreateTabMenuLogic::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateTabMenuLogic(args.renderable_scene()).execute(args);
};

CreateTabMenuLogic::CreateTabMenuLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateTabMenuLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string id = args.arguments.at<std::string>(KnownArgs::id);
    auto icon_widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::icon_left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::icon_right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::icon_bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::icon_top)));
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    size_t deflt = args.arguments.at<size_t>(KnownArgs::deflt);
    auto on_execute = args.arguments.try_at(KnownArgs::on_execute);
    // If the selection_ids array is not yet initialized, apply the default value.
    args.ui_focus.all_selection_ids.try_emplace(id, deflt);
    auto& tab_menu_logic = object_pool.create<TabMenuLogic>(
        CURRENT_SOURCE_LOCATION,
        "id = " + id,
        focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        args.confirm_button_press,
        args.gallery,
        ListViewStyle::ICON,
        args.arguments.at<std::string>(KnownArgs::selection_marker),
        args.arguments.path(KnownArgs::ttf_file),
        std::move(icon_widget),
        std::move(widget),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        args.external_json_macro_arguments,
        args.asset_references,
        args.ui_focus,
        args.num_renderings,
        args.button_states,
        args.ui_focus.menu_selection_ids[id],
        [macro_line_executor = args.macro_line_executor, on_execute]() {
            if (on_execute.has_value()) {
                macro_line_executor(*on_execute, nullptr, nullptr);
                // This results in a deadlock because both "delete_node_mutex" and "delete_rigid_body_mutex" are acquired.
                // std::scoped_lock rb_lock{ delete_rigid_body_mutex };
                // macro_line_executor(reload_transient_objects, nullptr);
            }
        });
    render_logics.append(
        { tab_menu_logic, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order, 0),
        CURRENT_SOURCE_LOCATION);
}
