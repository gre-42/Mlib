#include "Preload_Many.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <filesystem>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(json);
}

const std::string PreloadMany::key = "preload_many";

LoadSceneJsonUserFunction PreloadMany::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PreloadMany(args.renderable_scene()).execute(args);
};

PreloadMany::PreloadMany(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PreloadMany::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    primary_rendering_context.scene_node_resources.preload_many(args.arguments.path(KnownArgs::json));
}
