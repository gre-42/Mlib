#include "Execute_In_Physics_Thread.hpp"
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction ExecuteInPhysicsThread::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*execute_in_physics_thread"
        "\\s+([\\s\\S]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ExecuteInPhysicsThread(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ExecuteInPhysicsThread::ExecuteInPhysicsThread(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ExecuteInPhysicsThread::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string command = match[1].str();
    physics_set_fps.execute([mle=args.macro_line_executor, command, &rsc=args.rsc](){mle(command, nullptr, rsc);});
}
