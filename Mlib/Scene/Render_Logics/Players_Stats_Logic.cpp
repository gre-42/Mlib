#include "Players_Stats_Logic.hpp"
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

PlayersStatsLogic::PlayersStatsLogic(
    const Players& players,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    ScoreBoardConfiguration score_board_configuration)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  players_{players},
  score_board_configuration_{score_board_configuration}
{}

PlayersStatsLogic::~PlayersStatsLogic()
{}

void PlayersStatsLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    renderable_text().render(position_, players_.get_score_board(score_board_configuration_), {width, height}, line_distance_pixels_, true);  // true=periodic_position
}

void PlayersStatsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "PlayersStatsLogic\n";
}
