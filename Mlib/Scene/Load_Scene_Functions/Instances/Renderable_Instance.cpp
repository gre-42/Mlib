#include "Renderable_Instance.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction RenderableInstance::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*renderable_instance"
        "\\s+name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)"
        "(?:\\s+regex=(.*))?$");
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
    auto& node = scene.get_node(match[2].str());
    scene_node_resources.instantiate_renderable(
        match[3].str(),
        match[1].str(),
        node,
        { .regex = Mlib::compile_regex(match[4].str()) });
}
