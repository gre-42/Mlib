#include "Set_Dirtmap.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction SetDirtmap::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_dirtmap"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+offset=([\\w+-.]+)"
        "\\s+discreteness=([\\w+-.]+)"
        "\\s+scale=([\\w+-.]+)"
        "\\s+wrap_mode=(repeat|clamp_to_edge|clamp_to_border)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetDirtmap(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetDirtmap::SetDirtmap(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetDirtmap::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    dirtmap_logic.set_filename(args.fpath(match[1].str()).path);
    secondary_rendering_context.rendering_resources->set_offset("dirtmap", safe_stof(match[2].str()));
    secondary_rendering_context.rendering_resources->set_discreteness("dirtmap", safe_stof(match[3].str()));
    secondary_rendering_context.rendering_resources->set_scale("dirtmap", safe_stof(match[4].str()));
    secondary_rendering_context.rendering_resources->set_texture_wrap("dirtmap", wrap_mode_from_string(match[5].str()));
}
