#include "Append_Focuses.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

const std::string AppendFocuses::key = "append_focuses";

LoadSceneUserFunction AppendFocuses::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};

    std::scoped_lock lock{args.ui_focus.focuses.mutex};
    for (Focus focus : json_macro_arguments.get_vector<std::string>(focus_from_string)) {
        args.ui_focus.focuses.push_back(focus);
    }
};
