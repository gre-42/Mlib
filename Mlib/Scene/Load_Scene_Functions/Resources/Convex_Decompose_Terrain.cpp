#include "Convex_Decompose_Terrain.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(destination_physics_material);
DECLARE_ARGUMENT(depth);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(included_tags);
DECLARE_ARGUMENT(excluded_tags);
}

const std::string ConvexDecomposeTerrain::key = "create_barrier_triangle_hitboxes";

LoadSceneJsonUserFunction ConvexDecomposeTerrain::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().create_barrier_triangle_hitboxes(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource_name),
        args.arguments.at<float>(KnownArgs::depth),
        physics_material_from_string(args.arguments.at<std::string>(KnownArgs::destination_physics_material)),
        ColoredVertexArrayFilter{
            .included_tags = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::included_tags)),
            .excluded_tags = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::excluded_tags)),
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))});
};
