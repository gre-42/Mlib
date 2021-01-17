#include "Visual_Movable_3rd_Logger.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <sstream>

using namespace Mlib;

VisualMovable3rdLogger::VisualMovable3rdLogger(
    RenderLogic& scene_logic,
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    StatusWriter* status_writer,
    unsigned int log_components,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& offset,
    float font_height_pixels,
    float line_distance_pixels)
: scene_logic_{scene_logic},
  scene_node_{scene_node},
  advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components},
  offset_{offset},
  line_distance_pixels_{line_distance_pixels},
  ttf_filename_{ttf_filename},
  font_height_pixels_{font_height_pixels}
{
    scene_node.add_destruction_observer(this);
}

VisualMovable3rdLogger::~VisualMovable3rdLogger()
{}

void VisualMovable3rdLogger::notify_destroyed(void* destroyed_object) {
    advance_times_.schedule_delete_advance_time(this);
}

void VisualMovable3rdLogger::advance_time(float dt) {
    std::stringstream sstr;
    status_writer_->write_status(sstr, log_components_);
    text_ = sstr.str();
}

void VisualMovable3rdLogger::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (renderable_text_ == nullptr) {
        renderable_text_.reset(new RenderableText{ttf_filename_, font_height_pixels_});
    }
    FixedArray<float, 3> node_pos = scene_node_.absolute_model_matrix().t();
    auto position4 = dot1d(scene_logic_.vp(), homogenized_4(node_pos));
    if (position4(2) > scene_logic_.near_plane()) {
        FixedArray<float, 2> position2{
            position4(0) / position4(3) + offset_(0),
            -position4(1) / position4(3) - offset_(1)};
        auto p2 = (position2 * 0.5f + 0.5f) * FixedArray<float, 2>{(float)width, (float)height};
        renderable_text_->render(p2, text_, width, height, line_distance_pixels_);
    }
}
