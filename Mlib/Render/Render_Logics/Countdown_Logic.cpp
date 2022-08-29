#include "Countdown_Logic.hpp"
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

CountDownLogic::CountDownLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    Focuses& focuses,
    float nseconds)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  nseconds_{nseconds},
  focuses_{focuses}
{}

CountDownLogic::~CountDownLogic()
{}

void CountDownLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::lock_guard lock{focuses_.mutex};
    if (auto it = focuses_.find(Focus::COUNTDOWN_PENDING); it != focuses_.end()) {
        elapsed_time_ = std::chrono::duration<float>{0.f};
        *it = Focus::COUNTDOWN_COUNTING;
    }
    if (auto it = focuses_.find(Focus::COUNTDOWN_COUNTING); it != focuses_.end()) {
        if (focuses_.focus() == Focus::COUNTDOWN_COUNTING) {
            elapsed_time_ += std::chrono::duration<float>{render_config.dt};
        }
        if (elapsed_time_.count() >= nseconds_) {
            focuses_.erase(it);
        } else {
            renderable_text().render(
                position_,
                std::to_string((unsigned int)std::ceil(nseconds_ - elapsed_time_.count())),
                {width, height},
                line_distance_pixels_,
                true);  // true=periodic_position
        }
    }
}

void CountDownLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CountDownLogic\n";
}
