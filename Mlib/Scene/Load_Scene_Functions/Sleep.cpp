#include "Sleep.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SECONDS);

LoadSceneUserFunction Sleep::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*sleep"
        "\\s+seconds=(\\S+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        Sleep::execute(match);
        return true;
    } else {
        return false;
    }
};

void Sleep::execute(const Mlib::re::smatch& match)
{
    std::this_thread::sleep_for(std::chrono::duration<float>(safe_stof(match[SECONDS].str())));
}
