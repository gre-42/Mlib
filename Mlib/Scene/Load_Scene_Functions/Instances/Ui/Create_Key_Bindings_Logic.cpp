#include "Create_Key_Bindings_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings_Logic.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/Trim.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(section);
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(deflt);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

CreateKeyBindingsLogic::CreateKeyBindingsLogic(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateKeyBindingsLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto id = args.arguments.at<std::string>(KnownArgs::id);
    auto focus_filter = FocusFilter{
        .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        .submenu_ids = args.arguments.at<std::set<std::string>>(KnownArgs::submenus, { id }) };
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = args.arguments.at<std::string>(KnownArgs::title),
            .requires_ = args.arguments.at<std::vector<std::string>>(KnownArgs::required, std::vector<std::string>{})
        },
        focus_filter.focus_mask,
        args.arguments.at<size_t>(KnownArgs::deflt));
    auto& parameter_setter_logic = object_pool.create<KeyBindingsLogic>(
        CURRENT_SOURCE_LOCATION,
        "id = " + id,
        args.arguments.at<std::string>(KnownArgs::section),
        args.key_descriptions,
        args.key_configurations,
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        focus_filter,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),
        args.button_states,
        args.ui_focus.all_selection_ids.at(id));
    render_logics.append(
        { parameter_setter_logic, CURRENT_SOURCE_LOCATION },
        1 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "key_bindings",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateKeyBindingsLogic(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
