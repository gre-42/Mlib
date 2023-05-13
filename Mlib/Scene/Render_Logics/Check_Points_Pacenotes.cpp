#include "Check_Points_Pacenotes.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
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
    const std::string& ttf_filename,
    const FixedArray<float, 3>& color,
    const std::string& pacenotes_filename,
    const CheckPoints& check_points,
    size_t nlaps,
    double pacenotes_meters_read_ahead,
    double pacenotes_minimum_covered_meters,
    size_t pacenotes_maximum_number,
    RenderLogics& render_logics,
    AdvanceTimes& advance_times,
    SceneNode& moving_node)
: widget_distance_{widget_distance},
  text_widget_{std::move(text_widget)},
  picture_widget_{std::move(picture_widget)},
  font_height_{font_height},
  check_points_{&check_points},
  pacenote_reader_{pacenotes_filename, nlaps, pacenotes_meters_read_ahead, pacenotes_minimum_covered_meters},
  text_{ttf_filename, color},
  display_{gallery, text_, pictures_left, pictures_right},
  render_logics_{render_logics},
  advance_times_{advance_times},
  moving_node_{&moving_node}
{
    pacenotes_.reserve(pacenotes_maximum_number);
    moving_node_->clearing_observers.add(*this);
}

CheckPointsPacenotes::~CheckPointsPacenotes() {
    if (moving_node_ != nullptr) {
        moving_node_->clearing_observers.remove(*this);
    }
}

void CheckPointsPacenotes::advance_time(float dt) {
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

void CheckPointsPacenotes::notify_destroyed(const Object& destroyed_object) {
    check_points_ = nullptr;
    pacenotes_.clear();
    moving_node_ = nullptr;
    advance_times_.delete_advance_time(*this);
    render_logics_.remove(*this);
}

void CheckPointsPacenotes::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::shared_lock lock{mutex_};
    auto text_region = text_widget_->evaluate(lx, ly, YOrientation::AS_IS);
    auto picture_region = picture_widget_->evaluate(lx, ly, YOrientation::AS_IS);
    auto dx1 = widget_distance_.to_pixels(lx);
    float dx = 0.f;
    for (const auto& pacenote : pacenotes_) {
        display_.render(
            *pacenote,
            font_height_.to_pixels(ly),
            PixelRegion::transformed(*text_region, dx, 0.f),
            PixelRegion::transformed(*picture_region, dx, 0.f));
        dx += dx1;
    }
}

void CheckPointsPacenotes::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CheckPointsPacenotes\n";
}
