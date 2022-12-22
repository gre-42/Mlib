#include "Append_Focuses.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>
#include <mutex>

using namespace Mlib;

LoadSceneUserFunction AppendFocuses::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*append_focuses"
        "((?:\\s+(?:menu|loading|countdown_pending|scene|game_over))+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AppendFocuses::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::unique_lock lock{args.ui_focus.focuses.mutex};
    for (Focus focus : string_to_vector(match[1].str(), focus_from_string)) {
        args.ui_focus.focuses.push_back(focus);
    }
}
