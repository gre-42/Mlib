#include "Set_Focuses.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

const std::string SetFocuses::key = "set_focuses";

LoadSceneJsonUserFunction SetFocuses::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    execute(args);
};

void SetFocuses::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::scoped_lock lock{args.ui_focus.focuses.mutex};
    args.ui_focus.focuses.set_focuses(args.arguments.get_vector<std::string>(focus_from_string));
}
