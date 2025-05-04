#include "Smoothen_Edges.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Smoothness_Target.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(target);
DECLARE_ARGUMENT(smoothness);
DECLARE_ARGUMENT(niterations);
DECLARE_ARGUMENT(decay);
}

const std::string SmoothenEdges::key = "smoothen_edges";

LoadSceneJsonUserFunction SmoothenEdges::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().smoothen_edges(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        smoothness_target_from_string(args.arguments.at<std::string>(KnownArgs::target)),
        args.arguments.at<float>(KnownArgs::smoothness),
        args.arguments.at<size_t>(KnownArgs::niterations),
        args.arguments.at<float>(KnownArgs::decay));
};
