#include "Visual_Movable_3rd_Logger.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <sstream>

using namespace Mlib;

VisualMovable3rdLogger::VisualMovable3rdLogger(
    RenderLogic& scene_logic,
    DanglingRef<SceneNode> scene_node,
    AdvanceTimes& advance_times,
    StatusWriter& status_writer,
    StatusComponents log_components,
    std::string ttf_filename,
    const FixedArray<float, 2>& offset,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance)
: scene_logic_{scene_logic},
  scene_node_{scene_node.ptr()},
  advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components},
  offset_{offset},
  line_distance_{line_distance},
  ttf_filename_{std::move(ttf_filename)},
  font_height_{font_height}
{
    scene_node->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

VisualMovable3rdLogger::~VisualMovable3rdLogger() {
    if (scene_node_ != nullptr) {
        scene_node_->clearing_observers.remove(
            { *this, CURRENT_SOURCE_LOCATION },
            ObserverDoesNotExistBehavior::IGNORE);
    }
    advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void VisualMovable3rdLogger::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    scene_node_ = nullptr;
}

void VisualMovable3rdLogger::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    std::stringstream sstr;
    status_writer_.write_status(sstr, log_components_);
    text_ = sstr.str();
}

void VisualMovable3rdLogger::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VisualMovable3rdLogger::render");
    if (renderable_text_ == nullptr) {
        renderable_text_ = std::make_unique<TextResource>(
            ttf_filename_,
            FixedArray<float, 3>{1.f, 1.f, 1.f});
    }
    FixedArray<double, 3> node_pos = scene_node_->absolute_model_matrix().t();
    auto position4 = dot1d(scene_logic_.vp(), homogenized_4(node_pos));
    if (position4(2) > scene_logic_.near_plane()) {
        FixedArray<float, 2> position2{
            float(position4(0) / position4(3)) + offset_(0),
            -float(position4(1) / position4(3)) - offset_(1)};
        FixedArray<float, 2> size{lx.flength(), ly.flength()};
        auto p2 = (position2 * 0.5f + 0.5f) * size;
        renderable_text_->render(
            font_height_.to_pixels(ly),
            p2,
            size,
            (std::string)text_,
            AlignText::BOTTOM,
            line_distance_.to_pixels(ly));
    }
}

void VisualMovable3rdLogger::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VisualMovable3rdLogger\n";
}
