#include "Append_Focuses.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(content);
}

const std::string AppendFocuses::key = "append_focuses";

LoadSceneJsonUserFunction AppendFocuses::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{args.ui_focus.focuses.mutex};
    for (Focus focus : args.arguments.at_vector<std::string>(KnownArgs::content, focus_from_string)) {
        args.ui_focus.focuses.push_back(focus);
    }
};
