#include "Create_Video_Mode_Selector_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Verbose_Vector.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Video_Mode_Selector_Logic.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

using namespace Mlib;
namespace fs = std::filesystem;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(icon);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(user_id);
}

CreateVideoModeSelectorLogic::CreateVideoModeSelectorLogic(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateVideoModeSelectorLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto id = args.arguments.at<std::string>(KnownArgs::id);
    auto focus_filter = FocusFilter{
        .focus_mask = Focus::SETTINGS_MENU,
        .submenu_ids = { id } };
    ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title=args.arguments.at<std::string>(KnownArgs::title),
            .icon=args.arguments.at<std::string>(KnownArgs::icon)},
        focus_filter.focus_mask,
        0);
    auto user_id = args.arguments.at<uint32_t>(KnownArgs::user_id);
    auto& video_selector_logic = object_pool.create<VideoModeSelectorLogic>(
        CURRENT_SOURCE_LOCATION,
        std::move(id),
        args.window_logic,
        args.confirm_button_press.get(user_id),
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
        ui_focus,
        user_id);
    render_logics.append(
        { video_selector_logic, CURRENT_SOURCE_LOCATION },
        1 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "video_mode_selector",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateVideoModeSelectorLogic(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
