#include "Visual_Bullet_Count.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

VisualBulletCount::VisualBulletCount(
    ObjectPool& object_pool,
    AdvanceTimes& advance_times,
    RenderLogics& render_logics,
    const DanglingBaseClassRef<Player>& player,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    int z_order,
    FocusFilter focus_filter)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        font_color,
        font_height,
        line_distance }
    , on_player_delete_vehicle_internals_{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , advance_times_{ advance_times }
    , render_logics_{ render_logics }
    , player_{ player }
    , widget_{ std::move(widget) }
    , focus_filter_{ std::move(focus_filter) }
{
    advance_times_.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    on_player_delete_vehicle_internals_.add([this, &object_pool]() { object_pool.remove(*this); }, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *this, CURRENT_SOURCE_LOCATION }, z_order, CURRENT_SOURCE_LOCATION);
}

VisualBulletCount::~VisualBulletCount() {
    on_destroy.clear();
}

void VisualBulletCount::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{mutex_};
    if (!player_->has_gun_node()) {
        text_.clear();
        return;
    }
    text_ = "Ammo: " + std::to_string(player_->nbullets_available());
}

std::optional<RenderSetup> VisualBulletCount::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void VisualBulletCount::render_without_setup(
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
        if (ew_->result_may_have_changed()) {
            renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
        }
        renderable_text().render(
            font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
            *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
            text_,
            line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
            TextInterpolationMode::NEAREST_NEIGHBOR,
            GenericTextAlignment::DEFAULT,
            GenericTextAlignment::DEFAULT);
    }
}

bool VisualBulletCount::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void VisualBulletCount::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualBulletCount\n";
}
