#include "Check_Points_Pacenotes.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <mutex>

using namespace Mlib;

CheckPointsPacenotes::CheckPointsPacenotes(
    RenderLogicGallery& gallery,
    const std::vector<std::string>& pictures_left,
    const std::vector<std::string>& pictures_right,
    const ILayoutPixels& widget_distance,
    std::unique_ptr<IWidget>&& text_widget,
    std::unique_ptr<IWidget>&& picture_widget,
    const ILayoutPixels& font_height,
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    const FixedArray<float, 3>& font_color,
    const std::string& pacenotes_filename,
    const DanglingBaseClassRef<const CheckPoints>& check_points,
    size_t nlaps,
    double pacenotes_meters_read_ahead,
    double pacenotes_minimum_covered_meters,
    size_t pacenotes_maximum_number,
    FocusFilter focus_filter)
    : ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , widget_distance_{ widget_distance }
    , text_widget_{ std::move(text_widget) }
    , picture_widget_{ std::move(picture_widget) }
    , font_height_{ font_height }
    , check_points_{ check_points.ptr() }
    , pacenote_reader_{ pacenotes_filename, nlaps, pacenotes_meters_read_ahead, pacenotes_minimum_covered_meters }
    , text_{ ascii, std::move(ttf_filename), font_color }
    , display_{ gallery, text_, pictures_left, pictures_right }
    , focus_filter_{ std::move(focus_filter) }
    , on_destroy_check_points_{ check_points->on_destroy, CURRENT_SOURCE_LOCATION }
{
    pacenotes_.reserve(pacenotes_maximum_number);
    on_destroy_check_points_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

CheckPointsPacenotes::~CheckPointsPacenotes() {
    on_destroy.clear();
    pacenotes_.clear();
}

void CheckPointsPacenotes::advance_time(float dt, const StaticWorld& world) {
    if (check_points_ == nullptr) {
        return;
    }
    if (check_points_->has_meters_to_start()) {
        std::scoped_lock lock{mutex_};
        pacenotes_.clear();
        pacenote_reader_.read(
            check_points_->meters_to_start(),
            check_points_->lap_index(),
            pacenotes_);
    } else {
        pacenotes_.clear();
    }
    // if (pacenote_ != nullptr) {
    //     linfo() << *pacenote_;
    // }
}

std::optional<RenderSetup> CheckPointsPacenotes::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void CheckPointsPacenotes::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    auto text_region = text_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    auto picture_region = picture_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    auto dx1 = widget_distance_.to_pixels(lx, PixelsRoundMode::ROUND);
    float dx = 0.f;
    if (ew_->result_may_have_changed()) {
        text_.set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    {
        std::shared_lock lock{ mutex_ };
        for (const auto& pacenote : pacenotes_) {
            display_.render(
                *pacenote,
                font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
                TextInterpolationMode::NEAREST_NEIGHBOR,
                PixelRegion::transformed(*text_region, dx, 0.f),
                PixelRegion::transformed(*picture_region, dx, 0.f));
            dx += dx1;
        }
    }
}

FocusFilter CheckPointsPacenotes::focus_filter() const {
    return focus_filter_;
}

void CheckPointsPacenotes::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CheckPointsPacenotes\n";
}

void CheckPointsPacenotes::preload() const {
    display_.preload();
}
