#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(expected);
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "pop_focus",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                std::scoped_lock lock{args.ui_focus.focuses.mutex};
                if (auto e = args.arguments.try_at<std::string>(KnownArgs::expected); e.has_value()) {
                    if (!args.ui_focus.focuses.has_focus(single_focus_from_string(*e))) {
                        return;
                    }
                }
                args.ui_focus.focuses.pop_back();
            });
    }
} obj;

}
