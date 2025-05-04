#include "Gen_Grind_Lines.hpp"
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
DECLARE_ARGUMENT(source_name);
DECLARE_ARGUMENT(dest_name);
DECLARE_ARGUMENT(edge_angle);
DECLARE_ARGUMENT(averaged_normal_angle);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(included_tags);
DECLARE_ARGUMENT(excluded_tags);
}

const std::string GenGrindLines::key = "gen_grind_lines";

LoadSceneJsonUserFunction GenGrindLines::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().generate_grind_lines(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::source_name),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::dest_name),
        args.arguments.at<float>(KnownArgs::edge_angle) * degrees,
        args.arguments.at<float>(KnownArgs::averaged_normal_angle) * degrees,
        ColoredVertexArrayFilter{
            .included_tags = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::included_tags)),
            .excluded_tags = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::excluded_tags)),
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))});
};
