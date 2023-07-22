#include "Add_Foliage_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Modifiers/Add_Foliage_Resource.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(mesh_resource_name);
DECLARE_ARGUMENT(foliage_resource_name);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(near_grass_resource_names);
DECLARE_ARGUMENT(dirty_near_grass_resource_names);
DECLARE_ARGUMENT(near_grass_foliagemap);
DECLARE_ARGUMENT(near_grass_foliagemap_period);
DECLARE_ARGUMENT(near_grass_distance);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(up_axis);
}

const std::string AddFoliageResource::key = "add_foliage";

LoadSceneJsonUserFunction AddFoliageResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();

    auto parse_resource_name_func = [&scene_node_resources](const JsonMacroArguments& jma){
        return parse_resource_name(scene_node_resources, jma.get<std::string>());
    };
    
    add_foliage_resource(
        args.arguments.at<std::string>(KnownArgs::mesh_resource_name),
        args.arguments.at<std::string>(KnownArgs::foliage_resource_name),
        scene_node_resources,
        ColoredVertexArrayFilter{
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names))
        },
        args.arguments.children(KnownArgs::near_grass_resource_names, parse_resource_name_func),
        args.arguments.children(KnownArgs::dirty_near_grass_resource_names, parse_resource_name_func),
        args.arguments.at<double>(KnownArgs::near_grass_distance),
        args.arguments.path_or_variable(KnownArgs::near_grass_foliagemap).path,
        1.f / args.arguments.at<float>(KnownArgs::near_grass_foliagemap_period),
        args.arguments.at<float>(KnownArgs::scale, 1.f),
        up_axis_from_string(args.arguments.at<std::string>(KnownArgs::up_axis, "y")));
};
