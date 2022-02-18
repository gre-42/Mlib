#include "Register_Geographic_Mapping.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction RegisterGeographicMapping::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        
        "^\\s*register_geographic_mapping"
        "\\s+name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w-. \\(\\)/+-]+)");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RegisterGeographicMapping(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RegisterGeographicMapping::RegisterGeographicMapping(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RegisterGeographicMapping::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    auto& node = scene.get_node(match[2].str());
    args.scene_node_resources.register_geographic_mapping(
        match[3].str(),
        match[1].str(),
        node);

}
