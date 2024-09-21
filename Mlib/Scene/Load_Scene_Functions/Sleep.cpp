#include "Sleep.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(seconds);
}

const std::string Sleep::key = "sleep";

LoadSceneJsonUserFunction Sleep::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Sleep::execute(args);
};

void Sleep::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::this_thread::sleep_for(std::chrono::duration<double>(args.arguments.at<double>(KnownArgs::seconds)));
}
