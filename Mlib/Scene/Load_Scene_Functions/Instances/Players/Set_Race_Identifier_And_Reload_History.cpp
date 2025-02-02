#include "Set_Race_Identifier_And_Reload_History.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <filesystem>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(level_id);
DECLARE_ARGUMENT(time_of_day);
DECLARE_ARGUMENT(restrictions);
DECLARE_ARGUMENT(session);
DECLARE_ARGUMENT(laps);
DECLARE_ARGUMENT(milliseconds);
}

const std::string SetRaceIdentifierAndReloadHistory::key = "set_race_identifier_and_reload_history";

LoadSceneJsonUserFunction SetRaceIdentifierAndReloadHistory::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetRaceIdentifierAndReloadHistory(args.renderable_scene()).execute(args);
};

SetRaceIdentifierAndReloadHistory::SetRaceIdentifierAndReloadHistory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRaceIdentifierAndReloadHistory::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    players.set_race_identifier_and_reload_history(RaceIdentifier{
        .level = args.arguments.at<std::string>(KnownArgs::level_id),
        .time_of_day = args.arguments.at<std::string>(KnownArgs::time_of_day),
        .restrictions = args.arguments.at<std::string>(KnownArgs::restrictions),
        .session = args.arguments.at<std::string>(KnownArgs::session),
        .laps = args.arguments.at<size_t>(KnownArgs::laps),
        .milliseconds = args.arguments.at<uint64_t>(KnownArgs::milliseconds)});
}
