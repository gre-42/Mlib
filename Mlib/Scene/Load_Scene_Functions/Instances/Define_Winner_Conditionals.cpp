#include "Define_Winner_Conditionals.hpp"
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction DefineWinnerConditionals::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*define_winner_conditionals"
        "\\s+begin_rank=(\\d+)"
        "\\s+end_rank=(\\d+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        DefineWinnerConditionals(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

DefineWinnerConditionals::DefineWinnerConditionals(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void DefineWinnerConditionals::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    if (args.local_substitutions == nullptr) {
        throw std::runtime_error("Cannot define winner conditionals without local substitutions");
    }
    for (size_t rank = safe_stoz(match[1].str()); rank < safe_stoz(match[2].str()); ++rank) {
        LapTimeEventAndIdAndMfilename lapTimeEvent = players.get_winner_track_filename(rank);
        args.local_substitutions->merge(SubstitutionMap({
            {
                "IF_WINNER_RANK" + std::to_string(rank) + "_EXISTS",
                lapTimeEvent.m_filename.empty() ? "# " : ""
            },
            {
                "VEHICLE_WINNER" + std::to_string(rank),
                lapTimeEvent.event.vehicle
            },
            {
                "R_WINNER" + std::to_string(rank),
                std::to_string(lapTimeEvent.event.vehicle_color(0))
            },
            {
                "G_WINNER" + std::to_string(rank),
                std::to_string(lapTimeEvent.event.vehicle_color(1))
            },
            {
                "B_WINNER" + std::to_string(rank),
                std::to_string(lapTimeEvent.event.vehicle_color(2))
            }}));
    }

}
