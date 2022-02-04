#include "Create_Light.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Light.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateLight::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*light"
        "\\s+node=([\\w+-.]+)"
        "\\s+black_node=([\\w+-.]*)"
        "\\s+update=(once|always)"
        "\\s+with_depth_texture=(0|1)"
        "\\s+ambience=([\\w+-.]+)"
        "\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+shadow=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateLight(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateLight::CreateLight(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLight::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    std::lock_guard lock_guard{ delete_node_mutex };
    std::string node_name = match[1].str();
    SceneNode* node = scene.get_node(node_name);
    render_logics.prepend(node, std::make_shared<LightmapLogic>(
        read_pixels_logic,
        resource_update_cycle_from_string(match[3].str()),
        node_name,
        match[2].str(),               // black_node_name
        safe_stob(match[4].str())));  // with_depth_texture
    node->add_light(std::make_unique<Light>(Light{
        .ambience = {
            safe_stof(match[5].str()),
            safe_stof(match[6].str()),
            safe_stof(match[7].str())},
        .diffusivity = {
            safe_stof(match[8].str()),
            safe_stof(match[9].str()),
            safe_stof(match[10].str())},
        .specularity = {
            safe_stof(match[11].str()),
            safe_stof(match[12].str()),
            safe_stof(match[13].str())},
        .node_name = node_name,
        .only_black = !match[2].str().empty(),
        .shadow = safe_stob(match[14].str())}));

}
