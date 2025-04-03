#include "Merge_Meshes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Merge_Meshes.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/Group_And_Name.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(merged_name);
DECLARE_ARGUMENT(merged_physics_material);
}

const std::string MergeMeshes::key = "merge_meshes";

LoadSceneJsonUserFunction MergeMeshes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().add_modifier(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        [name = args.arguments.at<std::string>(KnownArgs::merged_name),
         physics_material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::merged_physics_material))]
        (ISceneNodeResource& resource)
        {
            for (const auto& acva : resource.get_rendering_arrays()) {
                merge_meshes(acva->scvas, name, Material{}, Morphology{ .physics_material = physics_material });
                merge_meshes(
                    acva->dcvas,
                    name,
                    Material{
                        .aggregate_mode = AggregateMode::ONCE,
                        .shading{
                            .ambient = {0.2f, 0.2f, 0.2f},
                            .diffuse = {0.2f, 0.2f, 0.2f},
                            .specular = {0.f, 0.f, 0.f}}},
                            Morphology{ .physics_material = physics_material });
            }
        });
};
