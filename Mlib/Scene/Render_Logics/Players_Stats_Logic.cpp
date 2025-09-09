#include "Players_Stats_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

PlayersStatsLogic::PlayersStatsLogic(
    const Players& players,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    ScoreBoardConfiguration score_board_configuration,
    FocusFilter focus_filter)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , players_{ players }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , score_board_configuration_{ score_board_configuration }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{}

PlayersStatsLogic::~PlayersStatsLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> PlayersStatsLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void PlayersStatsLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("PlayersStatsLogic::render");
    if (ew_->result_may_have_changed()) {
        renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        players_.get_score_board(score_board_configuration_),
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
}

bool PlayersStatsLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void PlayersStatsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "PlayersStatsLogic";
}
