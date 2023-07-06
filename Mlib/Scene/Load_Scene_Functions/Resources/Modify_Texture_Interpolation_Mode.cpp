#include "Modify_Texture_Interpolation_Mode.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Modifiers/Modify_Texture_Interpolation_Mode.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(magnifying_interpolation_mode);
DECLARE_ARGUMENT(included_names);
}

const std::string ModifyTextureInterpolationMode::key = "modify_texture_interpolation_mode";

LoadSceneJsonUserFunction ModifyTextureInterpolationMode::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    modify_texture_interpolation_mode(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        ColoredVertexArrayFilter{
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names))
        },
        interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::magnifying_interpolation_mode)));
};
