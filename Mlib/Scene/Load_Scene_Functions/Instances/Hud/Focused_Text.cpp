#include "Focused_Text.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Focused_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(text);
}

FocusedText::FocusedText(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void FocusedText::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto& loading_logic = object_pool.create<FocusedTextLogic>(
        CURRENT_SOURCE_LOCATION,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),        
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::font_color),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::position),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        args.arguments.at<std::string>(KnownArgs::text));
    render_logics.append(
        { loading_logic, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order),
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "focused_text",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                FocusedText(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
