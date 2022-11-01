#include "Start_Race.hpp"
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(READONLY);

LoadSceneUserFunction StartRace::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*start_race"
        "\\s+readonly=(\\S+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        StartRace(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

StartRace::StartRace(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void StartRace::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.start_race(RaceConfiguration{
        .readonly = safe_stob(match[READONLY].str())});
}
