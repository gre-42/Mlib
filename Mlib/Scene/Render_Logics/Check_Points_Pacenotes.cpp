#include "Check_Points_Pacenotes.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
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
    std::unique_ptr<IWidget>&& text_widget,
    std::unique_ptr<IWidget>&& picture_widget,
    const ILayoutPixels& font_height,
    const std::string& ttf_filename,
    const std::string& pacenotes_filename,
    const CheckPoints& check_points,
    size_t nlaps,
    size_t pacenotes_nread_ahead,
    RenderLogics& render_logics,
    AdvanceTimes& advance_times,
    SceneNode& moving_node)
: text_widget_{std::move(text_widget)},
  picture_widget_{std::move(picture_widget)},
  font_height_{font_height},
  check_points_{&check_points},
  pacenote_reader_{pacenotes_filename, nlaps, pacenotes_nread_ahead},
  pacenote_{nullptr},
  text_{ttf_filename},
  display_{gallery, text_, pictures_left, pictures_right},
  render_logics_{render_logics},
  advance_times_{advance_times},
  moving_node_{&moving_node}
{
    moving_node_->destruction_observers.add(*this);
}

CheckPointsPacenotes::~CheckPointsPacenotes() {
    if (moving_node_ != nullptr) {
        moving_node_->destruction_observers.remove(*this);
    }
}

void CheckPointsPacenotes::advance_time(float dt) {
    if (check_points_ == nullptr) {
        return;
    }
    std::scoped_lock lock{mutex_};
    pacenote_ = pacenote_reader_.read(
        check_points_->frame_index(),
        check_points_->lap_index());
    // if (pacenote_ != nullptr) {
    //     linfo() << *pacenote_;
    // }
}

void CheckPointsPacenotes::notify_destroyed(const Object& destroyed_object) {
    check_points_ = nullptr;
    pacenote_ = nullptr;
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
    if (pacenote_ == nullptr) {
        return;
    }
    display_.render(
        *pacenote_,
        font_height_.to_pixels(ly),
        *text_widget_->evaluate(lx, ly, YOrientation::AS_IS),
        *picture_widget_->evaluate(lx, ly, YOrientation::AS_IS));
}

void CheckPointsPacenotes::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "CheckPointsPacenotes\n";
}
