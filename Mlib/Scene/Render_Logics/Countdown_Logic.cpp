#include "Countdown_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <mutex>

using namespace Mlib;

CountDownLogic::CountDownLogic(
    AdvanceTimes& advance_times,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    float duration,
    Focus pending_focus,
    Focus counting_focus,
    std::string text,
    Focuses& focuses)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  advance_times_{advance_times},
  duration_{duration},
  pending_focus_{pending_focus},
  counting_focus_{counting_focus},
  text_{std::move(text)},
  focuses_{focuses}
{}

CountDownLogic::~CountDownLogic() = default;

void CountDownLogic::notify_destroyed(Object& destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this);
}

void CountDownLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("CountDownLogic::render");
    std::shared_lock lock{focuses_.mutex};
    if (focuses_.contains(counting_focus_)) {
        renderable_text().render(
            position_,
            {(float)width, (float)height},
            text_.empty()
                ? std::to_string((unsigned int)std::ceil((duration_ - elapsed_time_) / s))
                : text_,
            AlignText::BOTTOM,
            line_distance_pixels_);
    }
}

void CountDownLogic::advance_time(float dt) {
    std::unique_lock lock{focuses_.mutex};
    if (auto it = focuses_.find(pending_focus_); it != focuses_.end()) {
        elapsed_time_ = 0.f;
        *it = counting_focus_;
    }
    if (auto it = focuses_.find(counting_focus_); it != focuses_.end()) {
        if (focuses_.focus() == counting_focus_) {
            elapsed_time_ += dt;
        }
        if (elapsed_time_ >= duration_) {
            focuses_.erase(it);
        }
    }
}

void CountDownLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CountDownLogic\n";
}