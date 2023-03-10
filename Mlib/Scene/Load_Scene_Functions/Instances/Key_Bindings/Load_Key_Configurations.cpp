#include "Load_Key_Configurations.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(FALLBACK_FILENAME);

LoadSceneUserFunction LoadKeyConfigurations::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*load_key_configurations"
        "\\s+filename=([^\n]+)\n"
        "\\s+fallback_filename=(.+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        LoadKeyConfigurations(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
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
