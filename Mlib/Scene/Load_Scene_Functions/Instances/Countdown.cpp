#include "Countdown.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction Countdown::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*countdown"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+nseconds=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        Countdown(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

Countdown::Countdown(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Countdown::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto countdown_logic = std::make_shared<CountDownLogic>(
        args.fpath(match[1].str()).path,   // ttf_filename
        FixedArray<float, 2>{              // position
            safe_stof(match[2].str()),
            safe_stof(match[3].str())},
        safe_stof(match[4].str()),         // font_height_pixels
        safe_stof(match[5].str()),         // line_distance_pixels
        args.ui_focus.focuses,
        safe_stof(match[6].str()));        // nseconds
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, countdown_logic);

}
