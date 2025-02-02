#include "Start_Race.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(readonly);
}

const std::string StartRace::key = "start_race";

LoadSceneJsonUserFunction StartRace::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    StartRace(args.renderable_scene()).execute(args);
};

StartRace::StartRace(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void StartRace::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    players.start_race(RaceConfiguration{
        .readonly = args.arguments.at<bool>(KnownArgs::readonly)});
}
