#include "Create_Light_Only_Shadow.hpp"
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

LoadSceneUserFunction CreateLightOnlyShadow::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*light_only_shadow"
        "\\s+node=([\\w+-.]+)"
        "\\s+black_node=([\\w+-.]*)"
        "\\s+render_pass=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateLightOnlyShadow(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateLightOnlyShadow::CreateLightOnlyShadow(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLightOnlyShadow::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::lock_guard lock_guard{ delete_node_mutex };
    std::string node_name = match[NODE].str();
    auto& node = scene.get_node(node_name);
    ExternalRenderPassType render_pass = external_render_pass_type_from_string(match[EXTERNAL_RENDER_PASS].str());
    if ((render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_NODE))
    {
        throw std::runtime_error("Unsupported render pass type for \"only shadow\": " + match[EXTERNAL_RENDER_PASS].str());
    }
    render_logics.prepend(&node, std::make_shared<LightmapLogic>(
        read_pixels_logic,
        render_pass,
        node_name,
        match[BLACK_NODE].str(),                       // black_node_name
        false));                                       // with_depth_texture
    node.add_light(std::make_unique<Light>(Light{
        .ambience = {1.f, 1.f, 1.f},
        .diffusivity = {1.f, 1.f, 1.f},
        .specularity = {1.f, 1.f, 1.f},
        .node_name = node_name,
        .only_black = ((render_pass == ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) ||
                       (render_pass == ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES) ||
                       (render_pass == ExternalRenderPassType::LIGHTMAP_BLACK_NODE)),
        .shadow = true}));
}
