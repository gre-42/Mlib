#include "Repeat.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <iostream>

using namespace Mlib;

const std::string Repeat::key = "repeat";

LoadSceneUserFunction Repeat::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^([\\s\\S]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void Repeat::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    while (true) {
        std::cerr << "-";
        args.macro_line_executor(match[1].str(), args.local_substitutions);
    }
}
