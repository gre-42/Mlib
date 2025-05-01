#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(indicator);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "clear_requires_reload",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.ui_focus.clear_requires_reload(args.arguments.at<std::string>(KnownArgs::id));
                args.external_json_macro_arguments.set_and_notify(
                    args.arguments.at<std::string>(KnownArgs::indicator),
                    !args.ui_focus.requires_reload().empty());
            });
    }
} obj;

}
