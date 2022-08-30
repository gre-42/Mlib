#include "Set_Background_Color.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction SetBackgroundColor::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_background_color"
        "\\s+color=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetBackgroundColor(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetBackgroundColor::SetBackgroundColor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetBackgroundColor::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    FixedArray<float, 3> background_color{
        safe_stof(match[1].str()),
        safe_stof(match[2].str()),
        safe_stof(match[3].str())};
    standard_render_logic.set_background_color(background_color);
    post_processing_logic.set_background_color(background_color);
}
