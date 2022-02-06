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
        "\\s+alias=([\\w+-.]+)"
        "\\s+filenames=([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+) ([\\w-. \\(\\)/+-]+)$");
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    skybox_logic.set_filenames({
        args.fpath(match[2].str()).path,
        args.fpath(match[3].str()).path,
        args.fpath(match[4].str()).path,
        args.fpath(match[5].str()).path,
        args.fpath(match[6].str()).path,
        args.fpath(match[7].str()).path},
        match[1].str());
}
