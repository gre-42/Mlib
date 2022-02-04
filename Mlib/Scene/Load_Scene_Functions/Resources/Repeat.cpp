#include "Repeat.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <iostream>

using namespace Mlib;

LoadSceneUserFunction Repeat::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*repeat"
        "\\s+([\\s\\S]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void Repeat::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    while (true) {
        std::cerr << "-";
        args.macro_line_executor(match[1].str(), args.local_substitutions, args.rsc);
    }
}
