#include "Execute_In_Physics_Thread.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
}

const std::string ExecuteInPhysicsThread::key = "execute_in_physics_thread";

LoadSceneJsonUserFunction ExecuteInPhysicsThread::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ExecuteInPhysicsThread(args.physics_scene()).execute(args);
};

ExecuteInPhysicsThread::ExecuteInPhysicsThread(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ExecuteInPhysicsThread::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto command = args.arguments.at(KnownArgs::command);
    physics_set_fps.execute([mle=args.macro_line_executor, command](){mle(command, nullptr, nullptr);});
}
