#include "Set_Focuses.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(focuses);
}

const std::string SetFocuses::key = "set_focuses";

LoadSceneJsonUserFunction SetFocuses::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{args.ui_focus.focuses.mutex};
    args.ui_focus.focuses.set_focuses(args.arguments.at_vector_non_null<std::string>(KnownArgs::focuses, single_focus_from_string));
};
