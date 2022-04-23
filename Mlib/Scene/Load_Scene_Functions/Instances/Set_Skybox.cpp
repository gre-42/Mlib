#include "Set_Skybox.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction SetSkybox::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_skybox"
        "\\s+alias=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetSkybox(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetSkybox::SetSkybox(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSkybox::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    skybox_logic.set_alias(match[1].str());
}
