#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(alias);
DECLARE_ARGUMENT(filenames);
}
        
struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_cubemap",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                RenderingContextStack::primary_rendering_resources().add_cubemap(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::alias),
                    args.arguments.pathes_or_variables(KnownArgs::filenames, [](const FPath& p) { return VariableAndHash{ p.path }; }));
            });
    }
} obj;

}
