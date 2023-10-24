#include "Focused_Text.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Focused_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(text);
}

const std::string FocusedText::key = "focused_text";

LoadSceneJsonUserFunction FocusedText::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    FocusedText(args.renderable_scene()).execute(args);
};

FocusedText::FocusedText(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FocusedText::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto loading_logic = std::make_shared<FocusedTextLogic>(
        args.arguments.path(KnownArgs::ttf_file),
        FixedArray<float, 3>{1.f, 1.f, 1.f},
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::position),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        args.arguments.at<std::string>(KnownArgs::text));
    render_logics.append(
        nullptr,        // scene_node
        loading_logic,
        1);             // z_order
}
