#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <iostream>

using namespace Mlib;

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "repeat",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                while (true) {
                    lerr() << "-";
                    args.macro_line_executor(args.arguments.json(), args.local_json_macro_arguments);
                }
            });
    }
} obj;

}
