#include "Create_Light_Without_Shadow.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(ambience);
DECLARE_ARGUMENT(diffusivity);
DECLARE_ARGUMENT(specularity);
}

const std::string CreateLightWithoutShadow::key = "light_without_shadow";

LoadSceneJsonUserFunction CreateLightWithoutShadow::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateLightWithoutShadow(args.renderable_scene()).execute(args);
};

CreateLightWithoutShadow::CreateLightWithoutShadow(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLightWithoutShadow::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::scoped_lock lock_guard{ delete_node_mutex };
    std::string node_name = args.arguments.at<std::string>(KnownArgs::node);
    DanglingRef<SceneNode> node = scene.get_node(node_name, DP_LOC);
    node->add_light(std::make_unique<Light>(Light{
        .ambience = args.arguments.at<FixedArray<float, 3>>(KnownArgs::ambience),
        .diffusivity = args.arguments.at<FixedArray<float, 3>>(KnownArgs::diffusivity),
        .specularity = args.arguments.at<FixedArray<float, 3>>(KnownArgs::specularity),
        .resource_suffix = "",
        .shadow_render_pass = ExternalRenderPassType::NONE}));
}
