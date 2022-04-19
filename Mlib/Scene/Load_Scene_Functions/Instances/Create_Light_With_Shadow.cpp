#include "Create_Light_With_Shadow.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(BLACK_NODE);
DECLARE_OPTION(EXTERNAL_RENDER_PASS);
DECLARE_OPTION(WITH_DEPTH_TEXTURE);
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(DIFFUSIVITY_R);
DECLARE_OPTION(DIFFUSIVITY_G);
DECLARE_OPTION(DIFFUSIVITY_B);
DECLARE_OPTION(SPECULARITY_R);
DECLARE_OPTION(SPECULARITY_G);
DECLARE_OPTION(SPECULARITY_B);

LoadSceneUserFunction CreateLightWithShadow::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*light_with_shadow"
        "\\s+node=([\\w+-.]+)"
        "\\s+black_node=([\\w+-.]*)"
        "\\s+render_pass=(lightmap_global_static|lightmap_global_dynamic|lightmap_black_local_instances|lightmap_black_node)"
        "\\s+with_depth_texture=(0|1)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateLightWithShadow(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateLightWithShadow::CreateLightWithShadow(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLightWithShadow::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard lock_guard{ delete_node_mutex };
    std::string node_name = match[NODE].str();
    auto& node = scene.get_node(node_name);
    ExternalRenderPassType render_pass = external_render_pass_type_from_string(match[EXTERNAL_RENDER_PASS].str());
    if ((render_pass != ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC))
    {
        throw std::runtime_error("Unsupported render pass type for \"with shadow\": " + match[EXTERNAL_RENDER_PASS].str());
    }
    render_logics.prepend(&node, std::make_shared<LightmapLogic>(
        read_pixels_logic,
        render_pass,
        node_name,
        match[BLACK_NODE].str(),                       // black_node_name
        safe_stob(match[WITH_DEPTH_TEXTURE].str())));  // with_depth_texture
    node.add_light(std::make_unique<Light>(Light{
        .ambience = {
            safe_stof(match[AMBIENCE_R].str()),
            safe_stof(match[AMBIENCE_G].str()),
            safe_stof(match[AMBIENCE_B].str())},
        .diffusivity = {
            safe_stof(match[DIFFUSIVITY_R].str()),
            safe_stof(match[DIFFUSIVITY_G].str()),
            safe_stof(match[DIFFUSIVITY_B].str())},
        .specularity = {
            safe_stof(match[SPECULARITY_R].str()),
            safe_stof(match[SPECULARITY_G].str()),
            safe_stof(match[SPECULARITY_B].str())},
        .node_name = node_name,
        .shadow_render_pass = render_pass}));
}
