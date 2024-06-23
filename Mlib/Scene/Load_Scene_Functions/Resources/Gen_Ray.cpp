#include "Gen_Ray.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(from);
DECLARE_ARGUMENT(to);
}

const std::string GenRay::key = "gen_ray";

LoadSceneJsonUserFunction GenRay::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void GenRay::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().generate_ray(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::from),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::to));
}
