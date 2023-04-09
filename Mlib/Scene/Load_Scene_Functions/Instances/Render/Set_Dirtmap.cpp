#include "Set_Dirtmap.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(OFFSET);
DECLARE_OPTION(DISCRETENESS);
DECLARE_OPTION(SCALE);
DECLARE_OPTION(WRAP_MODE);

const std::string SetDirtmap::key = "set_dirtmap";

LoadSceneUserFunction SetDirtmap::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+offset=([\\w+-.]+)"
        "\\s+discreteness=([\\w+-.]+)"
        "\\s+scale=([\\w+-.]+)"
        "\\s+wrap_mode=(\\w+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetDirtmap(args.renderable_scene()).execute(match, args);
};

SetDirtmap::SetDirtmap(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetDirtmap::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    dirtmap_logic.set_filename(args.fpath(match[FILENAME].str()).path);
    secondary_rendering_context.rendering_resources->set_offset("dirtmap", safe_stof(match[OFFSET].str()));
    secondary_rendering_context.rendering_resources->set_discreteness("dirtmap", safe_stof(match[DISCRETENESS].str()));
    secondary_rendering_context.rendering_resources->set_scale("dirtmap", safe_stof(match[SCALE].str()));
    secondary_rendering_context.rendering_resources->set_texture_wrap("dirtmap", wrap_mode_from_string(match[WRAP_MODE].str()));
}
