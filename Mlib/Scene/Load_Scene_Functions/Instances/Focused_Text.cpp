#include "Focused_Text.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Focused_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

LoadSceneUserFunction FocusedText::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*focused_text"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+focus_mask=(none|base|menu|loading|countdown_any|scene|game_over|always)"
        "\\s+text=(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        FocusedText(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

FocusedText::FocusedText(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FocusedText::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto loading_logic = std::make_shared<FocusedTextLogic>(
        args.fpath(match[1].str()).path,   // ttf_filename
        FixedArray<float, 2>{              // position
            safe_stof(match[2].str()),
            safe_stof(match[3].str())},
        safe_stof(match[4].str()),         // font_height_pixels
        safe_stof(match[5].str()),         // line_distance_pixels
        focus_from_string(match[6].str()), // focus mask
        match[7].str());                   // text
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, loading_logic);
}
