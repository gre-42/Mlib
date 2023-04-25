#include "Downsample.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(factor);
}

const std::string Downsample::key = "downsample";

LoadSceneJsonUserFunction Downsample::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void Downsample::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.scene_node_resources.downsample(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<size_t>(KnownArgs::factor));
}
