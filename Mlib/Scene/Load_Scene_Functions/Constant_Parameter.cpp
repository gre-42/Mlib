#include "Constant_Parameter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(value);
}

const std::string ConstantParameter::key = "constant_parameter";

LoadSceneJsonUserFunction ConstantParameter::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    args.external_json_macro_arguments.set_and_notify(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at(KnownArgs::value));
};
