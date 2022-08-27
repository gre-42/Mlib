#include "Preload.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(DIR);

LoadSceneUserFunction Preload::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*preload"
        "\\s+json=([\\w+-. \\(\\)/\\\\:]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        Preload(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

Preload::Preload(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Preload::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    primary_rendering_context.scene_node_resources.preload_many(args.fpath(match[DIR].str()).path);
}
