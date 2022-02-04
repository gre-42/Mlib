#include "Players_Stats.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Players_Stats_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction PlayersStats::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*players_stats"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+score_board=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayersStats(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayersStats::PlayersStats(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayersStats::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto players_stats_logic = std::make_shared<PlayersStatsLogic>(
        players,
        args.fpath(match[1].str()).path,                      // ttf_filename
        FixedArray<float, 2>{                                 // position
            safe_stof(match[2].str()),
            safe_stof(match[3].str())},
        safe_stof(match[4].str()),                            // font_height_pixels
        safe_stof(match[5].str()),                            // line_distance_pixels
        (ScoreBoardConfiguration)safe_stoi(match[6].str()));  // score board configuration
    render_logics.append(nullptr, players_stats_logic);
}
