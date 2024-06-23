#include "Gen_Triangle_Rays.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(npoints);
DECLARE_ARGUMENT(lengths);
DECLARE_ARGUMENT(delete_triangles);
}

const std::string GenTriangleRays::key = "gen_triangle_rays";

LoadSceneJsonUserFunction GenTriangleRays::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void GenTriangleRays::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().generate_triangle_rays(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<size_t>(KnownArgs::npoints),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::lengths),
        args.arguments.at<bool>(KnownArgs::delete_triangles));
}
