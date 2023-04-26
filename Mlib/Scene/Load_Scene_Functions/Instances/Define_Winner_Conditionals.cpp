#include "Define_Winner_Conditionals.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(begin_rank);
DECLARE_ARGUMENT(end_rank);
}

const std::string DefineWinnerConditionals::key = "define_winner_conditionals";

LoadSceneJsonUserFunction DefineWinnerConditionals::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    DefineWinnerConditionals(args.renderable_scene()).execute(args);
};

DefineWinnerConditionals::DefineWinnerConditionals(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DefineWinnerConditionals::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (args.local_json_macro_arguments == nullptr) {
        THROW_OR_ABORT("Cannot define winner conditionals without local substitutions");
    }
    for (size_t rank = args.arguments.at<size_t>(KnownArgs::begin_rank); rank < args.arguments.at<size_t>(KnownArgs::end_rank); ++rank) {
        LapTimeEventAndIdAndMfilename lapTimeEvent = players.get_winner_track_filename(rank);
        args.local_json_macro_arguments->merge(JsonMacroArguments(nlohmann::json{
            {
                "IF_WINNER_RANK" + std::to_string(rank) + "_EXISTS",
                lapTimeEvent.m_filename.empty()
            },
            {
                "VEHICLE_WINNER" + std::to_string(rank),
                lapTimeEvent.event.vehicle
            },
            {
                "COLOR_WINNER" + std::to_string(rank),
                lapTimeEvent.event.vehicle_color
            }}));
    }
}
