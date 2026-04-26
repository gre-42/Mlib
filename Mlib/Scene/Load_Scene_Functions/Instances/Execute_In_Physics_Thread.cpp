#include "Execute_In_Physics_Thread.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
}

ExecuteInPhysicsThread::ExecuteInPhysicsThread(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ExecuteInPhysicsThread::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto command = args.arguments.at(KnownArgs::command);
    physics_set_fps.execute([mle=args.macro_line_executor, command](){mle(command, nullptr);});
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "execute_in_physics_thread",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ExecuteInPhysicsThread{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
