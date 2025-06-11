#include "Repeat.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <iostream>

using namespace Mlib;

const std::string Repeat::key = "repeat";

LoadSceneJsonUserFunction Repeat::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    execute(args);
};

void Repeat::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    while (true) {
        lerr() << "-";
        args.macro_line_executor(args.arguments.json(), args.local_json_macro_arguments);
    }
}
