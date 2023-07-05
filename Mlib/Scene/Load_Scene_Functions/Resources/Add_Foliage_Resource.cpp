#include "Add_Foliage_Resource.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Modifiers/Add_Foliage_Resource.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(mesh_resource_name);
DECLARE_ARGUMENT(foliage_resource_name);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(up);
}

const std::string AddFoliageResource::key = "add_foliage";

LoadSceneJsonUserFunction AddFoliageResource::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    add_foliage_resource(
        args.arguments.at<std::string>(KnownArgs::mesh_resource_name),
        args.arguments.at<std::string>(KnownArgs::foliage_resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        ColoredVertexArrayFilter{
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names))
        },
        args.arguments.at<float>(KnownArgs::scale, 1.f),
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::up));
};
