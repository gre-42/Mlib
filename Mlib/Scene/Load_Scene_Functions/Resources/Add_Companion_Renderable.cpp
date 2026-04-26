#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(companion_resource);
DECLARE_ARGUMENT(regex);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_companion_renderable",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                RenderingContextStack::primary_scene_node_resources().add_companion(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::companion_resource),
                    RenderableResourceFilter{
                        .cva_filter = {
                            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::regex, "")) }});
            });
    }
} obj;

}
