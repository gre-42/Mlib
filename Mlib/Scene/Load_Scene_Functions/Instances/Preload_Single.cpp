#include "Preload_Single.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <filesystem>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
}

const std::string PreloadSingle::key = "preload_single";

LoadSceneJsonUserFunction PreloadSingle::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PreloadSingle(args.renderable_scene()).execute(args);
};

PreloadSingle::PreloadSingle(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PreloadSingle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    primary_rendering_context.scene_node_resources.preload_single(
        args.arguments.at<std::string>(KnownArgs::resource),
        RenderableResourceFilter{});
}
