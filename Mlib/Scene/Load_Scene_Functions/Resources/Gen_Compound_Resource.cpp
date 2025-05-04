#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Compound_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(source_names);
DECLARE_ARGUMENT(dest_name);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "compound_resource",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                RenderingContextStack::primary_scene_node_resources().add_resource(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::dest_name),
                    std::make_shared<CompoundResource>(
                        RenderingContextStack::primary_scene_node_resources(),
                        args.arguments.at_non_null<std::vector<VariableAndHash<std::string>>>(KnownArgs::source_names, std::vector<VariableAndHash<std::string>>{})));
            });
    }
} obj;

}
