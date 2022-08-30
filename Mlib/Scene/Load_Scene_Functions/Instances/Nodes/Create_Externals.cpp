#include "Create_Externals.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(EXTERNALS_MODE);

LoadSceneUserFunction CreateExternals::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_externals"
        "\\s+player=([\\w+-.]+)"
        "\\s+mode=(\\w+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateExternals(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateExternals::CreateExternals(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateExternals::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.get_player(match[PLAYER].str()).create_externals(
        externals_mode_from_string(match[EXTERNALS_MODE].str()));
}
