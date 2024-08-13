#include "Globals.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

const std::string Globals::key = "globals";

LoadSceneJsonUserFunction Globals::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.external_json_macro_arguments.merge_and_notify(args.arguments);
};
