#include "Set_User_Count.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user_count);
}

void SetUserCount::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    args.remote_sites.set_local_user_count(args.arguments.at<uint32_t>(KnownArgs::user_count));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_user_count",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetUserCount::execute(args);
            });
    }
} obj;

}
