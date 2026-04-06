#include <Mlib/Geometry/Cameras/Ortho_Camera_Config.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera_Config_Json.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Color_Extrapolation_mode.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Render_Resource_To_Textures_Lazy.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color_texture_name);
DECLARE_ARGUMENT(depth_texture_name);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(ortho_camera_config);
DECLARE_ARGUMENT(texture_size);
DECLARE_ARGUMENT(nsamples_msaa);
DECLARE_ARGUMENT(dpi);
DECLARE_ARGUMENT(color_extrapolation_mode);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "render_resource_to_textures",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
                auto& rendering_resources = RenderingContextStack::primary_rendering_resources();
                auto depth_texture_name = args.arguments.try_at_non_null<VariableAndHash<std::string>>(KnownArgs::depth_texture_name);
                render_resource_to_textures_lazy(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::color_texture_name),
                    depth_texture_name.value_or(VariableAndHash<std::string>{}),    // depth_texture_name
                    scene_node_resources,
                    rendering_resources,
                    args.arguments.at<OrthoCameraConfig>(KnownArgs::ortho_camera_config),
                    depth_texture_name.has_value()                                  // depth_kind
                        ? FrameBufferChannelKind::TEXTURE
                        : FrameBufferChannelKind::ATTACHMENT,
                    args.arguments.at<EFixedArray<int, 2>>(KnownArgs::texture_size),
                    args.arguments.at<int>(KnownArgs::nsamples_msaa),
                    args.arguments.at<float>(KnownArgs::dpi),
                    ColorExtrapolationMode::ENABLED);
            });
    }
} obj;

}
