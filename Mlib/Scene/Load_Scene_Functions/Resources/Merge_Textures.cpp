#include "Merge_Textures.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Modifiers/Merge_Textures.hpp>
#include <Mlib/Render/Modifiers/Merged_Textures_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(merged_resource_name);
DECLARE_ARGUMENT(merged_texture_name);
DECLARE_ARGUMENT(merged_array_name);
DECLARE_ARGUMENT(merged_blend_mode);
DECLARE_ARGUMENT(merged_aggregate_mode);
DECLARE_ARGUMENT(merged_max_triangle_distance);
DECLARE_ARGUMENT(merged_cull_faces);
}

const std::string MergeBlendedMaterials::key = "merge_textures";

LoadSceneJsonUserFunction MergeBlendedMaterials::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    auto rendering_resources = RenderingContextStack::primary_rendering_resources();
    scene_node_resources.add_modifier(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        [mesh_resource_name = args.arguments.at<std::string>(KnownArgs::resource_name),
         mcfg = MergedTexturesConfig{
            .resource_name = args.arguments.at<std::string>(KnownArgs::merged_resource_name),
            .array_name = args.arguments.at<std::string>(KnownArgs::merged_array_name),
            .texture_name = args.arguments.at<std::string>(KnownArgs::merged_texture_name),
            .blend_mode = blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::merged_blend_mode)),
            .aggregate_mode = aggregate_mode_from_string(args.arguments.at<std::string>(KnownArgs::merged_aggregate_mode)),
            .max_triangle_distance = args.arguments.at<float>(KnownArgs::merged_max_triangle_distance),
            .cull_faces = args.arguments.at<bool>(KnownArgs::merged_cull_faces)
         },
         &scene_node_resources = scene_node_resources,
         rendering_resources = rendering_resources]
        (ISceneNodeResource& resource)
        {
            merge_textures(
                mesh_resource_name,
                mcfg,
                scene_node_resources,
                *rendering_resources);
        });
};
