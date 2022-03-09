#include "Clear_Parameters.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction ClearParameters::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*clear_parameters$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ClearParameters(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ClearParameters::ClearParameters(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearParameters::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.external_substitutions.clear();
}
