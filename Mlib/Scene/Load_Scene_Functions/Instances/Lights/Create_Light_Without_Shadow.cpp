#include "Create_Light_Without_Shadow.hpp"
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
DECLARE_OPTION(AMBIENCE_R);
DECLARE_OPTION(AMBIENCE_G);
DECLARE_OPTION(AMBIENCE_B);
DECLARE_OPTION(DIFFUSIVITY_R);
DECLARE_OPTION(DIFFUSIVITY_G);
DECLARE_OPTION(DIFFUSIVITY_B);
DECLARE_OPTION(SPECULARITY_R);
DECLARE_OPTION(SPECULARITY_G);
DECLARE_OPTION(SPECULARITY_B);

LoadSceneUserFunction CreateLightWithoutShadow::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*light_without_shadow"
        "\\s+node=([\\w+-.]+)"
        "\\s+ambience=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+diffusivity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+specularity=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateLightWithoutShadow(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateLightWithoutShadow::CreateLightWithoutShadow(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLightWithoutShadow::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::scoped_lock lock_guard{ delete_node_mutex };
    std::string node_name = match[NODE].str();
    auto& node = scene.get_node(node_name);
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
        .resource_suffix = "",
        .shadow_render_pass = ExternalRenderPassType::NONE}));
}
