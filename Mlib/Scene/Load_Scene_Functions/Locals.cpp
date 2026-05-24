#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "locals",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                if (args.local_json_macro_arguments == nullptr) {
                    throw std::runtime_error("Local JSON macro arguments not set");
                }
                args.local_json_macro_arguments->merge(args.arguments);
            });
    }
} obj;

}
