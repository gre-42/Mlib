#include "Add_Companion_Renderable.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(companion_resource);
DECLARE_ARGUMENT(regex);
}

const std::string AddCompanionRenderable::key = "add_companion_renderable";

LoadSceneJsonUserFunction AddCompanionRenderable::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddCompanionRenderable::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().add_companion(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::companion_resource),
        RenderableResourceFilter{
            .cva_filter = {
                .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::regex, "")) }});
}
