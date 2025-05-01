#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(reason);
DECLARE_ARGUMENT(indicator);
}
        
struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_requires_reload",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                args.ui_focus.set_requires_reload(
                    args.arguments.at<std::string>(KnownArgs::id),
                    args.macro_line_executor.eval<std::string>(args.arguments.at<std::string>(KnownArgs::reason)));
                args.external_json_macro_arguments.set_and_notify(
                    args.arguments.at<std::string>(KnownArgs::indicator),
                    !args.ui_focus.requires_reload().empty());
            });
    }
} obj;

}
