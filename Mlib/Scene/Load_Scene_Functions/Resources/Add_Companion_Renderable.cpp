#include "Add_Companion_Renderable.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

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
        match[1].str(),
        match[2].str(),
        { .regex = Mlib::compile_regex(match[3].str()) });
}
