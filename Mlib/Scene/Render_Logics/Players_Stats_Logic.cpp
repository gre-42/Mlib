#include "Players_Stats_Logic.hpp"
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

PlayersStatsLogic::PlayersStatsLogic(
    const Players& players,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  players_{players}
{}

PlayersStatsLogic::~PlayersStatsLogic()
{}

void PlayersStatsLogic::initialize(GLFWwindow* window) {
    RenderTextLogic::initialize(window);
}

void PlayersStatsLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    assert_true(window_ != nullptr);
    renderable_text_->render(position_, players_.get_score_board(), width, height, line_distance_pixels_, true);  // true=periodic_position
}

float PlayersStatsLogic::near_plane() const {
    throw std::runtime_error("PlayersStatsLogic::requires_postprocessing not implemented");
}

float PlayersStatsLogic::far_plane() const {
    throw std::runtime_error("PlayersStatsLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& PlayersStatsLogic::vp() const {
    throw std::runtime_error("PlayersStatsLogic::vp not implemented");
}

bool PlayersStatsLogic::requires_postprocessing() const {
    throw std::runtime_error("PlayersStatsLogic::requires_postprocessing not implemented");
}
