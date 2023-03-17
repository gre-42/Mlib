#include "Players_Stats_Logic.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

PlayersStatsLogic::PlayersStatsLogic(
    const Players& players,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    ScoreBoardConfiguration score_board_configuration)
: RenderTextLogic{
    ttf_filename,
    {1.f, 1.f, 1.f},
    font_height,
    line_distance},
  players_{players},
  score_board_configuration_{score_board_configuration},
  widget_{std::move(widget)}
{}

PlayersStatsLogic::~PlayersStatsLogic() = default;

void PlayersStatsLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("PlayersStatsLogic::render");
    renderable_text().render(
        font_height_.to_pixels(ly),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS),
        players_.get_score_board(score_board_configuration_),
        line_distance_.to_pixels(ly));
}

void PlayersStatsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "PlayersStatsLogic\n";
}
