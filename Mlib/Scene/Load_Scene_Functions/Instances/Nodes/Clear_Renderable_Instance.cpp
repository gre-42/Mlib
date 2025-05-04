#include "Clear_Renderable_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(name);
}

const std::string ClearRenderableInstance::key = "clear_renderable_instance";

LoadSceneJsonUserFunction ClearRenderableInstance::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ClearRenderableInstance(args.renderable_scene()).execute(args);
};

ClearRenderableInstance::ClearRenderableInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearRenderableInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene
    .get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC)
    ->clear_renderable_instance(VariableAndHash{ args.arguments.at<std::string>(KnownArgs::name) });
}
