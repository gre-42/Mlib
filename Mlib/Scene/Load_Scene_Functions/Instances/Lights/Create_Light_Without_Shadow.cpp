#include "Create_Light_Without_Shadow.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(diffuse);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(fresnel_ambient);
DECLARE_ARGUMENT(fog_ambient);
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
    std::string node_name = args.arguments.at<std::string>(KnownArgs::node);
    DanglingRef<SceneNode> node = scene.get_node(node_name, DP_LOC);
    node->add_light(std::make_unique<Light>(Light{
        .ambient = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::ambient),
        .diffuse = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::diffuse),
        .specular = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::specular),
        .fresnel_ambient = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::fresnel_ambient),
        .fog_ambient = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::fog_ambient),
        .shadow_render_pass = ExternalRenderPassType::NONE}));
}
