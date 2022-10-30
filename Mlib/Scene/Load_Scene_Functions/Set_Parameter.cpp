#include "Set_Parameter.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(VALUE);

LoadSceneUserFunction SetParameter::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_parameter"
        "\\s+(\\w+):(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetParameter::execute(match, args);
        return true;
    } else {
        return false;
    }
};

void SetParameter::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.external_substitutions.insert(match[NAME].str(), match[VALUE].str());
}
