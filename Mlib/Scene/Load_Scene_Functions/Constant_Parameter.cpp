#include "Constant_Parameter.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(VALUE);

const std::string ConstantParameter::key = "constant_parameter";

LoadSceneUserFunction ConstantParameter::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^(\\w+):(\\S*)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ConstantParameter::execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ConstantParameter::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.external_substitutions.set_and_notify(match[NAME].str(), match[VALUE].str());
}
