#include "Gen_Contour_Edges.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(source_name);
DECLARE_ARGUMENT(dest_name);
}

const std::string GenContourEdges::key = "gen_contour_edges";

LoadSceneJsonUserFunction GenContourEdges::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().generate_contour_edges(
        args.arguments.at<std::string>(KnownArgs::source_name),
        args.arguments.at<std::string>(KnownArgs::dest_name));
};
