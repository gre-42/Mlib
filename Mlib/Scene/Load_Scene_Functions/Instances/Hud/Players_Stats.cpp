#include "Players_Stats.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Players_Stats_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(SCORE_BOARD);

LoadSceneUserFunction PlayersStats::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*players_stats"
        "\\s+z_order=(\\d+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,
        .rendering_resources = primary_rendering_context.rendering_resources,
        .z_order = safe_stoi(match[Z_ORDER].str())} }; 
    auto players_stats_logic = std::make_shared<PlayersStatsLogic>(
        players,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()),
        (ScoreBoardConfiguration)safe_stoi(match[SCORE_BOARD].str()));
    render_logics.append(nullptr, players_stats_logic);
}
