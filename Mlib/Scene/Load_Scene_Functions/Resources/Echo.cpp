#include "Echo.hpp"
#include <Mlib/Scene/User_Function_Args.hpp>
#include <iostream>

using namespace Mlib;

LoadSceneUserFunction Echo::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*echo"
        "\\s+([\\S\\s]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void Echo::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::cerr << match[1].str() << std::endl;
}