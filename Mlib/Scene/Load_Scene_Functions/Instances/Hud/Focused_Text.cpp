#include "Focused_Text.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Focused_Text_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(TEXT);

LoadSceneUserFunction FocusedText::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*focused_text"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+focus_mask=(\\w+)"
        "\\s+text=(.*)$");
    Mlib::re::smatch match;
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
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()),
        focus_from_string(match[FOCUS_MASK].str()),
        match[TEXT].str());
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = 1} };
    render_logics.append(nullptr, loading_logic);
}
