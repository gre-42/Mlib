#include "Renderable_Instance.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);
DECLARE_OPTION(INCLUDE);
DECLARE_OPTION(EXCLUDE);

LoadSceneUserFunction RenderableInstance::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*renderable_instance"
        "\\s+name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)"
        "(?:\\s+include=(.*?))?"
        "(?:\\s+exclude=(.*?))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RenderableInstance(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RenderableInstance::RenderableInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RenderableInstance::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene_node_resources.instantiate_renderable(
        match[RESOURCE].str(),
        match[NAME].str(),
        scene.get_node(match[NODE].str()),
        { .include = Mlib::compile_regex(match[INCLUDE].str()),
          .exclude = match[EXCLUDE].matched
            ? Mlib::compile_regex(match[EXCLUDE].str())
            : Mlib::compile_regex("$ ^") });
}
