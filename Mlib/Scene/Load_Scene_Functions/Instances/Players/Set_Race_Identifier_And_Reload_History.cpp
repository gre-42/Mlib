#include "Set_Race_Identifier_And_Reload_History.hpp"
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <filesystem>

using namespace Mlib;
namespace fs = std::filesystem;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SESSION);
DECLARE_OPTION(LAPS);
DECLARE_OPTION(MILLISECONDS);

LoadSceneUserFunction SetRaceIdentifierAndReloadHistory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_race_identifier_and_reload_history"
        "\\s+session=(\\S+)"
        "\\s+laps=(\\S+)"
        "\\s+milliseconds=(\\S+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetRaceIdentifierAndReloadHistory(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetRaceIdentifierAndReloadHistory::SetRaceIdentifierAndReloadHistory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRaceIdentifierAndReloadHistory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.set_race_identifier_and_reload_history(RaceIdentifier{
        .level = fs::path{args.script_filename}.stem().string(),
        .session = match[SESSION].str(),
        .laps = safe_stoz(match[LAPS].str()),
        .milliseconds = safe_stou64(match[MILLISECONDS].str())});
}