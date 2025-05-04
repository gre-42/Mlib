#include "Modify_Rendering_Material.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Modifiers/Modify_Rendering_Material.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);

DECLARE_ARGUMENT(blend_mode);
DECLARE_ARGUMENT(occluded_pass);
DECLARE_ARGUMENT(occluder_pass);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(histogram);

DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

const std::string ModifyRenderingMaterial::key = "modify_rendering_material";

LoadSceneJsonUserFunction ModifyRenderingMaterial::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    modify_rendering_material(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        ColoredVertexArrayFilter{
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))
        },
        args.arguments.contains(KnownArgs::blend_mode)
            ? std::optional{ blend_mode_from_string(args.arguments.at<std::string>(KnownArgs::blend_mode)) }
            : std::nullopt,
        args.arguments.contains(KnownArgs::occluded_pass)
            ? std::optional{ external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluded_pass)) }
            : std::nullopt,
        args.arguments.contains(KnownArgs::occluder_pass)
            ? std::optional{ external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::occluder_pass)) }
            : std::nullopt,
        args.arguments.contains(KnownArgs::magnifying_interpolation_mode)
            ? std::optional{ interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::magnifying_interpolation_mode)) }
            : std::nullopt,
        args.arguments.contains(KnownArgs::histogram)
            ? std::optional{ args.arguments.path_or_variable(KnownArgs::histogram).path }
            : std::nullopt);
};
