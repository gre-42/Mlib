#include "Set_Race_Configuration_And_Reload_History.hpp"
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SESSION);
DECLARE_OPTION(LAPS);
DECLARE_OPTION(MILLISECONDS);
DECLARE_OPTION(READONLY);

LoadSceneUserFunction SetRaceConfigurationAndReloadHistory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_race_configuration_and_reload_history"
        "\\s+session=(\\S+)"
        "\\s+laps=(\\S+)"
        "\\s+milliseconds=(\\S+)"
        "\\s+readonly=(\\S+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRaceConfigurationAndReloadHistory(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetRaceConfigurationAndReloadHistory::SetRaceConfigurationAndReloadHistory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRaceConfigurationAndReloadHistory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.set_race_configuration_and_reload_history(RaceConfiguration{
        .session = match[SESSION].str(),
        .laps = safe_stoz(match[LAPS].str()),
        .milliseconds = safe_stou64(match[MILLISECONDS].str()),
        .readonly = safe_stob(match[READONLY].str())});
}
