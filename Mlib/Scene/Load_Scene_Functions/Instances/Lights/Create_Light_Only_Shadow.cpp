#include "Create_Light_Only_Shadow.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Lightmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(black_node);
DECLARE_ARGUMENT(render_pass);
DECLARE_ARGUMENT(lightmap_width);
DECLARE_ARGUMENT(lightmap_height);
DECLARE_ARGUMENT(smooth_niterations);
}

const std::string CreateLightOnlyShadow::key = "light_only_shadow";

LoadSceneJsonUserFunction CreateLightOnlyShadow::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateLightOnlyShadow(args.renderable_scene()).execute(args);
};

CreateLightOnlyShadow::CreateLightOnlyShadow(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateLightOnlyShadow::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto render_pass = external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::render_pass));
    if ((render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_MOVABLES) &&
        (render_pass != ExternalRenderPassType::LIGHTMAP_BLACK_NODE))
    {
        THROW_OR_ABORT("Unsupported render pass type for \"only shadow\": " + args.arguments.at<std::string>(KnownArgs::render_pass));
    }
    auto light = std::make_shared<Light>(Light{
        .ambient = {1.f, 1.f, 1.f},
        .diffuse = {1.f, 1.f, 1.f},
        .specular = {1.f, 1.f, 1.f},
        .fresnel_ambient = {1.f, 1.f, 1.f},
        .fog_ambient = {1.f, 1.f, 1.f},
        .lightmap_color = nullptr,
        .lightmap_depth = nullptr,
        .shadow_render_pass = render_pass});
    auto& o = global_object_pool.create<LightmapLogic>(
        CURRENT_SOURCE_LOCATION,
        rendering_resources,
        read_pixels_logic,
        render_pass,
        node,
        light,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::black_node), // black_node_name
        false,                                                                  // with_depth_texture
        args.arguments.at<int>(KnownArgs::lightmap_width),
        args.arguments.at<int>(KnownArgs::lightmap_height),
        args.arguments.at<UFixedArray<uint32_t, 2>>(KnownArgs::smooth_niterations));
    o.on_node_clear.add([&o]() { global_object_pool.remove(o); }, CURRENT_SOURCE_LOCATION);
    render_logics.prepend(
        { o, CURRENT_SOURCE_LOCATION },
        0 /* z_order */,
        CURRENT_SOURCE_LOCATION);
    node->add_light(light);
}
