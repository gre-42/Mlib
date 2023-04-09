#include "Players_Stats.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Physics/Score_Board_Configuration.hpp>
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
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(SCORE_BOARD);

const std::string PlayersStats::key = "players_stats";

LoadSceneUserFunction PlayersStats::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^z_order=(\\d+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+left=([\\w+-.]+)"
        "\\s+right=([\\w+-.]+)"
        "\\s+bottom=([\\w+-.]+)"
        "\\s+top=([\\w+-.]+)"
        "\\s+font_height=(\\w+)"
        "\\s+line_distance=(\\w+)"
        "\\s+score_board=([\\w|]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    PlayersStats(args.renderable_scene()).execute(match, args);
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
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
        args.layout_constraints.get_pixels(match[LINE_DISTANCE].str()),
        score_board_configuration_from_string(match[SCORE_BOARD].str()));
    render_logics.append(nullptr, players_stats_logic);
}
