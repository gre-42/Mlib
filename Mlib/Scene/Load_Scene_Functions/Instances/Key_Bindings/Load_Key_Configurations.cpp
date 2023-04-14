#include "Load_Key_Configurations.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(FALLBACK_FILENAME);

const std::string LoadKeyConfigurations::key = "load_key_configurations";

LoadSceneUserFunction LoadKeyConfigurations::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^filename=([^\n]+)\n"
        "\\s+fallback_filename=(.+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    LoadKeyConfigurations(args.renderable_scene()).execute(match, args);
};

LoadKeyConfigurations::LoadKeyConfigurations(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void LoadKeyConfigurations::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.load_key_configurations(
        args.fpath(match[FILENAME].str()).path,
        args.fpath(match[FALLBACK_FILENAME].str()).path);
}
