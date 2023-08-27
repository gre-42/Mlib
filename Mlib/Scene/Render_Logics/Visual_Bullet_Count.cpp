#include "Visual_Bullet_Count.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>

using namespace Mlib;

VisualBulletCount::VisualBulletCount(
    AdvanceTimes& advance_times,
    Player& player,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
: RenderTextLogic{
    ttf_filename,
    {1.f, 1.f, 1.f},
    font_height,
    line_distance},
  advance_times_{advance_times},
  player_{player},
  widget_{std::move(widget)}
{}

VisualBulletCount::~VisualBulletCount() = default;

void VisualBulletCount::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    advance_times_.delete_advance_time(*this);
}

void VisualBulletCount::advance_time(float dt) {
    std::scoped_lock lock{mutex_};
    if (!player_.has_gun_node()) {
        text_.clear();
        return;
    }
    text_ = "Ammo: " + std::to_string(player_.nbullets_available());
}

void VisualBulletCount::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualBulletCount::render");
    std::scoped_lock lock{mutex_};
    if (!text_.empty()) {
        renderable_text().render(
            font_height_.to_pixels(ly),
            *widget_->evaluate(lx, ly, YOrientation::AS_IS),
            text_,
            line_distance_.to_pixels(ly));
    }
}

void VisualBulletCount::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualBulletCount\n";
}
