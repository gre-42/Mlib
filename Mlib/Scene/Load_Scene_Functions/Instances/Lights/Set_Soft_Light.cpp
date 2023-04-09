#include "Set_Soft_Light.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

const std::string SetSoftLight::key = "set_soft_light";

LoadSceneUserFunction SetSoftLight::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^filename=([\\w+-. \\(\\)/\\\\:]*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetSoftLight(args.renderable_scene()).execute(match, args);
};

SetSoftLight::SetSoftLight(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSoftLight::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    post_processing_logic.set_soft_light_filename(args.fpath(match[1].str()).path);
}
