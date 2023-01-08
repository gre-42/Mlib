#include "Visual_Bullet_Count.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

VisualBulletCount::VisualBulletCount(
    AdvanceTimes& advance_times,
    Player& player,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height,
    float line_distance,
    ScreenUnits units)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height,
    line_distance,
    units},
  advance_times_{advance_times},
  player_{player},
  size_{size}
{}

VisualBulletCount::~VisualBulletCount() = default;

void VisualBulletCount::notify_destroyed(Object& destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this);
}

void VisualBulletCount::advance_time(float dt) {
    std::lock_guard lock{mutex_};
    if (!player_.has_gun_node()) {
        text_.clear();
        return;
    }
    text_ = "Ammo: " + std::to_string(player_.nbullets_available());
}

void VisualBulletCount::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualBulletCount::render");
    std::lock_guard lock{mutex_};
    if (!text_.empty()) {
        renderable_text().render({width, height}, {xdpi, ydpi}, position_, size_, text_, line_distance_);
    }
}

void VisualBulletCount::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualBulletCount\n";
}
