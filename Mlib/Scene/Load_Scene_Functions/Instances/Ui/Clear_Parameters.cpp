#include "Clear_Parameters.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

const std::string ClearParameters::key = "clear_parameters";

LoadSceneUserFunction ClearParameters::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    ClearParameters(args.renderable_scene()).execute(match, args);
};

ClearParameters::ClearParameters(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ClearParameters::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    THROW_OR_ABORT("clear_parameters is not implemented");
    // args.external_substitutions.clear_and_notify();
}
