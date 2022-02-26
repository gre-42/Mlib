#include "Add_Companion_Renderable.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RESOURCE);
DECLARE_OPTION(COMPANION_RESOURCE);
DECLARE_OPTION(REGEX);

LoadSceneUserFunction AddCompanionRenderable::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_companion_renderable"
        "\\s+resource=([\\w+-.]+)"
        "\\s+companion_resource=([\\w+-.]+)"
        "(?:\\s+regex=(.*))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddCompanionRenderable::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.add_companion(
        match[RESOURCE].str(),
        match[COMPANION_RESOURCE].str(),
        RenderableResourceFilter{
            .resource_filter = {
                .include = Mlib::compile_regex(match[REGEX].str()) }});
}
