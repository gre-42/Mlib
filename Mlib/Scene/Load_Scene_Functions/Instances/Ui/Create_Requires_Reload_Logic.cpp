#include "Create_Requires_Reload_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Reload_Required.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(style);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(focus_mask);
}

CreateRequiresReloadLogic::CreateRequiresReloadLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRequiresReloadLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    auto& requires_reload_logic = object_pool.create<ReloadRequired>(
        CURRENT_SOURCE_LOCATION,
        args.arguments.at<std::string>(KnownArgs::title),
        args.arguments.at<std::string>(KnownArgs::charset, *ascii),
        args.arguments.path(KnownArgs::ttf_file),
        std::move(widget),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        FocusFilter{focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask))},
        args.macro_line_executor,
        args.ui_focus);
    render_logics.append(
        { requires_reload_logic, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order, 0),
        CURRENT_SOURCE_LOCATION);
}


namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "reload_required",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateRequiresReloadLogic(args.renderable_scene()).execute(args);            
            });
    }
} obj;

}
