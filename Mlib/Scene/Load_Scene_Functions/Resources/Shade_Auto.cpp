#include "Shade_Auto.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Modifiers/Add_Shade_Auto_Modifier.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(seam_angle);
}

const std::string ShadeAuto::key = "shade_auto";

LoadSceneJsonUserFunction ShadeAuto::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    add_shade_auto_modifier(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        ColoredVertexArrayFilter{
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))
        },
        args.arguments.at<float>(KnownArgs::seam_angle) * degrees);
};
