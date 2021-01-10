#include "Visual_Movable_Logger.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <sstream>

using namespace Mlib;

VisualMovableLogger::VisualMovableLogger(
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    StatusWriter* status_writer,
    unsigned int log_components,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels)
: RenderTextLogic{
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels},
  advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components}
{
    scene_node.add_destruction_observer(this);
}

VisualMovableLogger::~VisualMovableLogger()
{}

void VisualMovableLogger::notify_destroyed(void* destroyed_object) {
    advance_times_.schedule_delete_advance_time(this);
}

void VisualMovableLogger::advance_time(float dt) {
    std::stringstream sstr;
    status_writer_->write_status(sstr, log_components_);
    text_ = sstr.str();
}

void VisualMovableLogger::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    renderable_text().render(position_, text_, width, height, line_distance_pixels_, true);  // true=periodic_position
}

float VisualMovableLogger::near_plane() const {
    throw std::runtime_error("VisualMovableLogger::requires_postprocessing not implemented");
}

float VisualMovableLogger::far_plane() const {
    throw std::runtime_error("VisualMovableLogger::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& VisualMovableLogger::vp() const {
    throw std::runtime_error("VisualMovableLogger::vp not implemented");
}

const TransformationMatrix<float>& VisualMovableLogger::iv() const {
    throw std::runtime_error("VisualMovableLogger::iv not implemented");
}

bool VisualMovableLogger::requires_postprocessing() const {
    throw std::runtime_error("VisualMovableLogger::requires_postprocessing not implemented");
}
