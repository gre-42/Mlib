#include "Countdown_Logic.hpp"
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

CountDownLogic::CountDownLogic(
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    std::list<Focus>& focus,
    float nseconds)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  timeout_started_{false},
  nseconds_{nseconds},
  focus_{focus}
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
    if (auto it = std::find(focus_.begin(), focus_.end(), Focus::COUNTDOWN); it != focus_.end()) {
        if (!timeout_started_) {
            start_time_ = std::chrono::steady_clock::now();
            timeout_started_ = true;
        }
        std::chrono::duration<float> elapsed_time = std::chrono::steady_clock::now() - start_time_;
        if (elapsed_time.count() >= nseconds_) {
            focus_.erase(it);
        } else {
            renderable_text().render(
                position_,
                std::to_string((unsigned int)std::ceil(nseconds_ - elapsed_time.count())),
                width,
                height,
                line_distance_pixels_,
                true);  // true=periodic_position
        }
    }
}
