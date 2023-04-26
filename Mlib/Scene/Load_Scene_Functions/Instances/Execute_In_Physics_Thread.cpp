#include "Execute_In_Physics_Thread.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
DECLARE_ARGUMENT(capture);
}

const std::string ExecuteInPhysicsThread::key = "execute_in_physics_thread";

LoadSceneJsonUserFunction ExecuteInPhysicsThread::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ExecuteInPhysicsThread(args.renderable_scene()).execute(args);
};

ExecuteInPhysicsThread::ExecuteInPhysicsThread(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ExecuteInPhysicsThread::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto command = args.arguments.at(KnownArgs::command);
    auto a = args.arguments.contains(KnownArgs::capture)
        ? args.arguments.child(KnownArgs::capture)
        : JsonMacroArguments();
    physics_set_fps.execute([mle=args.macro_line_executor, a, command](){mle(command, &a, nullptr);});
}
