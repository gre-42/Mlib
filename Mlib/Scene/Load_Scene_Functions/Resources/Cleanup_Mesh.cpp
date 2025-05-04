#include "Cleanup_Mesh.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Modifiers/Add_Cleanup_Mesh_Modifier.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(min_vertex_distance);
DECLARE_ARGUMENT(min_distance_material_filter);
DECLARE_ARGUMENT(modulo_uv);
}

const std::string CleanupMesh::key = "cleanup_mesh";

LoadSceneJsonUserFunction CleanupMesh::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto min_distance_filter = PhysicsMaterial::NONE;
    auto min_vertex_distance = args.arguments.at<float>(KnownArgs::min_vertex_distance, 0.f);
    if (min_vertex_distance != 0.f) {
        min_distance_filter = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::min_distance_material_filter));
    }
    add_cleanup_mesh_modifier(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        min_vertex_distance,
        min_distance_filter,
        args.arguments.at<bool>(KnownArgs::modulo_uv, true));
};
