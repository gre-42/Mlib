#include "Set_Background_Color.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(R);
DECLARE_OPTION(G);
DECLARE_OPTION(B);

const std::string SetBackgroundColor::key = "set_background_color";

LoadSceneUserFunction SetBackgroundColor::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^color=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetBackgroundColor(args.renderable_scene()).execute(match, args);
};

SetBackgroundColor::SetBackgroundColor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetBackgroundColor::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    FixedArray<float, 3> background_color{
        safe_stof(match[R].str()),
        safe_stof(match[G].str()),
        safe_stof(match[B].str())};
    standard_render_logic.set_background_color(background_color);
    post_processing_logic.set_background_color(background_color);
}
