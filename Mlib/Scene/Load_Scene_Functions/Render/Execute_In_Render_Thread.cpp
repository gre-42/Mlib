#include "Execute_In_Render_Thread.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
DECLARE_ARGUMENT(capture);
}

const std::string ExecuteInRenderThread::key = "execute_in_render_thread";

LoadSceneJsonUserFunction ExecuteInRenderThread::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto command = args.arguments.at(KnownArgs::command);
    auto a = args.arguments.contains(KnownArgs::capture)
        ? args.arguments.child(KnownArgs::capture)
        : JsonMacroArguments();
    args.render_set_fps.set_fps.execute([mle=args.macro_line_executor, a, command](){mle(command, &a, nullptr);});
};
