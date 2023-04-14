#include "Execute_In_Physics_Thread.hpp"
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>

using namespace Mlib;

const std::string ExecuteInPhysicsThread::key = "execute_in_physics_thread";

LoadSceneUserFunction ExecuteInPhysicsThread::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^([\\s\\S]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    ExecuteInPhysicsThread(args.renderable_scene()).execute(match, args);
};

ExecuteInPhysicsThread::ExecuteInPhysicsThread(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ExecuteInPhysicsThread::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string command = match[1].str();
    physics_set_fps.execute([mle=args.macro_line_executor, command](){mle(command, nullptr);});
}
